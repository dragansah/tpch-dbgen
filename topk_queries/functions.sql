-- Function: histogram_array(anyarray)

-- DROP FUNCTION histogram_array(anyarray);

CREATE OR REPLACE FUNCTION histogram_array(anyarray)
  RETURNS text[] AS
$BODY$
    SELECT regexp_split_to_array(substring($1::text from 2 for (length($1::text) - 2)), ',')
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;
ALTER FUNCTION histogram_array(anyarray)
  OWNER TO postgres;


-- Function: generate_where_clause(text, text[])

-- DROP FUNCTION generate_where_clause(text, text[]);

CREATE OR REPLACE FUNCTION generate_where_clause(attr_name text, histogram text[])
  RETURNS text AS
$BODY$
DECLARE
    where_clauses text[];
    where_clause text;
    n numeric;
BEGIN
	if histogram is null then
		return where_clauses;
	end if;

	n = array_length(histogram, 1);
	for i in 1 .. n
	loop
		where_clause = attr_name || ' >= ' || histogram[n - i + 1];
		where_clauses = array_append(where_clauses, where_clause);	
	end loop;
    
    RETURN where_clauses;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION generate_where_clause(text, text[])
  OWNER TO postgres;


-- Function: generate_analyze(text[], text[], integer, text)

-- DROP FUNCTION generate_analyze(text[], text[], integer, text);

CREATE OR REPLACE FUNCTION generate_analyze(tables text[], attrs text[], topk_rows integer, query text)
  RETURNS text AS
$BODY$
DECLARE
    hist_lengths numeric[];
    where_clauses text[];
    where_count numeric[];
    hist_bounds text[];
    where_clause text;
    n numeric;
    declare r record;
    top_row_explain_plan text;
    sql_query text;
    rows_explain_plan integer;
    cost_explain_plan text;
    result text[];
BEGIN

	if array_length(attrs, 1) <> array_length(tables, 1)
	then 
		raise 'Tables and attrs lengths must be the same';
	end if;

	n = array_length(attrs, 1);
	for i in 1 .. n loop
		where_count[i] = 1; -- init
		select 
		generate_where_clause(attrs[i], histogram_array(histogram_bounds)) 
		into hist_bounds
		from pg_stats 
		where tablename = tables[i] and attname = attrs[i];
		hist_lengths[i] = array_length(hist_bounds, 1);
		where_clauses = array_cat(where_clauses, hist_bounds);
	end loop;

	rows_explain_plan = 0;
	while rows_explain_plan < topk_rows loop
		where_clause = '';
		-- append where clauses
		for i in 1 .. n loop
			where_clause = where_clause || where_clauses[(i-1) * hist_lengths[i] + where_count[i]];
			if i < n then
				where_clause = where_clause || ' and ';
			end if;
			where_count[i] = where_count[i] + 1;
		end loop;
		-- execute the where_clause
		sql_query = query || where_clause || ' limit ' || topk_rows;
		for r in execute 'explain ' || sql_query
		loop
			top_row_explain_plan = r."QUERY PLAN";
			exit;
		end loop;

		cost_explain_plan = 
		(string_to_array(
			(string_to_array(top_row_explain_plan, 'cost=')::text[])[2],
			' '))[1];

		for r in execute 'select count(*) as num_rows from (' || sql_query || ') temp'
		loop
			rows_explain_plan = r."num_rows";
			exit;
		end loop;
		RAISE INFO 'rows: % cost:%', rows_explain_plan, cost_explain_plan;
		result = array_append(result, 'rows: ' || rows_explain_plan || ' cost:' || cost_explain_plan || ' query:' || sql_query);
	end loop;	

    RETURN result;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION generate_analyze(text[], text[], integer, text)
  OWNER TO postgres;


-- Function: expl(text)

-- DROP FUNCTION expl(text);

CREATE OR REPLACE FUNCTION expl(q text)
  RETURNS SETOF text AS
$BODY$
declare r record;
begin
for r in execute 'explain ' || q loop
return next r."QUERY PLAN";
end loop;
end$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION expl(text)
  OWNER TO postgres;


