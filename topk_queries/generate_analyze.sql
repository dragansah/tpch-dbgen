-- Function: generate_analyze(text[], text[], integer, text)

-- DROP FUNCTION generate_analyze(text[], text[], integer, text);

CREATE OR REPLACE FUNCTION generate_analyze(tables text[], attrs text[], topk_rows integer, histogram_scaling_factor integer[], join_intervals boolean, query text)
  RETURNS text AS
$BODY$
DECLARE
    hist_lengths numeric[];
    where_clauses text[];
    hist_points_2_dim numeric[];
    where_count numeric[];
    hist_bounds text[];
    hist_bounds_scaled text[];
    where_clause text;
    n numeric;
    declare r record;
    top_row_explain_plan text;
    sql_query text;
    row_count integer;
    cost_explain_plan text;
    F_max numeric;
    F_max_temp numeric;
    F_clause text;
    result text[];
				record_temp record;
BEGIN

	if array_length(attrs, 1) <> array_length(tables, 1)
	then 
		raise 'Tables and attrs lengths must be the same';
	end if;

	n = array_length(attrs, 1);
	for i in 1 .. n loop
		select array_reverse(histogram_array(histogram_bounds))
		into   hist_bounds
		from   pg_stats 
		where  tablename = tables[i] and attname = attrs[i];

		-- transform hist_bounds according to the histogram scaling factor
		for j in 1 .. array_length(histogram_scaling_factor) loop
			if histogram_scaling_factor[j] is not null then
				for k in 1 .. (array_length(hist_bounds) /  histogram_scaling_factor[j]) loop
					 hist_bounds_scaled = array_cat(hist_bounds_scaled, hist_bounds[k * histogram_scaling_factor[j]]);
				end loop;
				if hist_bounds_scaled[array_length(hist_bounds_scaled)] <> hist_bounds[array_length(hist_bounds)] then
					 hist_bounds_scaled = array_cat(hist_bounds_scaled, hist_bounds[array_length(hist_bounds)]);
				end if;
			end if;
		end loop;

		where_clauses = array_cat(where_clauses, generate_where_clause(attrs[i], hist_bounds));		
		hist_lengths[i] = array_length(hist_bounds, 1);
		hist_points_2_dim = array_cat(hist_points_2_dim, hist_bounds::numeric[]);
		RAISE INFO '%.% hist_bounds:%', tables[i], attrs[i], hist_bounds;	
	end loop;
	
	if join_intervals = true then
		record_temp = join_intervals_with_same_f_max(hist_points_2_dim, hist_lengths);
		hist_points_2_dim = record_temp.hist_points_2_dim;
		hist_lengths = record_temp.hist_lengths;
	end if;

	raise info 'The 2 dim histogram: %', hist_points_2_dim;
	
	for i in 1 .. n loop where_count[i] = 0; end loop;
	row_count = 0;

	while row_count < topk_rows loop

		-- append and clauses in the where clause
		where_clause = '';
		for i in 1 .. n loop
			where_count[i] = where_count[i] + 1;
			where_clause = where_clause || where_clauses[(i-1) * hist_lengths[i] + where_count[i]];
			if i < n then where_clause = where_clause || ' and '; end if;			
		end loop;
		sql_query = query || where_clause ;

		-- compute F_max
		F_max = null;
		for k in 1 .. n loop	
			F_max_temp = 0;
			for i in 1 .. n loop
				if i = k then
					F_max_temp = F_max_temp + hist_points_2_dim[(i-1) * hist_lengths[i] + where_count[i]];
				else
					F_max_temp = F_max_temp + hist_points_2_dim[(i-1) * hist_lengths[i] + 1];
				end if;
				if F_max < F_max_temp or F_max is null then F_max = F_max_temp; end if;
			end loop;
		end loop;

		F_clause = ' and (';
		for i in 1 .. n loop
			F_clause = F_clause || tables[i] || '.' || attrs[i];
			if i < n then F_clause = F_clause || ' + '; end if;
		end loop;
		F_clause = F_clause || ' ) >= ' || F_max;
		sql_query = sql_query || F_clause;

		raise info 'F_max = %', F_max;
		
		-- what is the cost of the query computed by the optimizer
		for r in execute 'explain ' || sql_query
		loop
			top_row_explain_plan = r."QUERY PLAN";
			exit;
		end loop;
		cost_explain_plan = 
		(string_to_array(
			(string_to_array(top_row_explain_plan, 'cost=')::text[])[2],
			' '))[1];

		-- now whats the real row count of the query, because the optimizer is not that accurate
		for r in execute 'select count(*) as num_rows from (' || sql_query || ') temp'
		loop
			row_count = r."num_rows";
			exit;
		end loop;	
		RAISE INFO 'rows: % cost:% query:%', row_count, cost_explain_plan, sql_query;
		result = array_append(result, 'rows: ' || row_count || ' cost:' || cost_explain_plan || ' query:' || sql_query);
	end loop;	

    RETURN result;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION generate_analyze(text[], text[], integer, text)
  OWNER TO postgres;
