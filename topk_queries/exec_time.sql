
	with recursive t(n) as(
	select 1
	union all
	select n+1 from t where n < 10
)
select exec_time('q1_10', 'select * from
		lineitem,
		orders,
		part
	where
	o_orderkey = l_orderkey and
	p_partkey = l_partkey
	order by (l_extendedprice + o_totalprice + p_retailprice) desc
	LIMIT 10') from t