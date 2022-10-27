use yzmon;	/* 把yzmon设置为缺省数据库 */

/* 查看当前数据库中的表，应该只有6张表 */
show tables;
show function status where db = "yzmon";
show procedure status  where db = "yzmon";

/* 建立第1小题的view（此处仅为demo，完成作业时删除建立新的即可）*/
/* 注意性能 */
drop view if exists view_inner_zj_bid2num;	/* 如果视图已存在则先删除 */
create view view_inner_zj_bid2num
as
select branch1_id as bid1, branch1_name as name, bid2num
from branch1 natural join (select branch2_branch1_id as branch1_id, COUNT(*) as bid2num
from branch2
group by branch1_id) as subx;




/* 建立第2小题的view */
drop view if exists view_inner_zj_devnum;
create view view_inner_zj_devnum
as
select branch1_id as bid1, branch1_name as name, COUNT(*) as devnum
from branch1 natural join branch2 join devorg
where branch1_id = branch2_branch1_id and branch2_id = devorg_branch2_id
group by branch1_id, name;



/* 建立第3小题的view */


/*临时视图1*/
drop view if exists view_inner_zj_devcount_sub1;
create view view_inner_zj_devcount_sub1
as
select distinct branch1_id as bid1, branch1_name as name, devorg_id
from branch1 natural join branch2 natural join devorg natural join devstate_base
where branch1_id = branch2_branch1_id and
branch2_id = devorg_branch2_id and
devorg.devorg_id = devstate_base.devstate_base_devid;

/*对计划单列市单独建临时视图2（找4位数）*/
drop view if exists view_inner_zj_devcount_sub2;
create view view_inner_zj_devcount_sub2
as
select bid1
from view_inner_zj_devcount_sub1
where bid1 like '____';

/*先统计计划单列市*/
drop view if exists view_inner_zj_devcount_sub3;
create view view_inner_zj_devcount_sub3
as
select bid1, name, count(*) as devcount
from view_inner_zj_devcount_sub1
where bid1 in (select bid1 from view_inner_zj_devcount_sub2) and bid1 = left(devorg_id, 4)
group by bid1, name;

/*再统计两位数（查询结果直接减去）*/


/*某省所有计划单列市的dev数（做被减数）*/
drop view if exists view_inner_zj_devcount_sub4;
create view view_inner_zj_devcount_sub4
as
select left(bid1, 2) as four_id, count(*) as four_count
from view_inner_zj_devcount_sub3
group by left(bid1, 2);


drop table if exists view_inner_zj_devcount_sub5;
create table view_inner_zj_devcount_sub5
as
select bid1, name, count(*) as devcount
from view_inner_zj_devcount_sub1
group by bid1, name;

/*update view_inner_zj_devcount_sub5
set devcount = devcount - (select devcount from view_inner_zj_devcount_sub4 where four_id = bid1)
where bid1 in (select four_id from view_inner_zj_devcount_sub4);*/

/*建立真正的视图3*/
drop view if exists view_inner_zj_devcount;
create view view_inner_zj_devcount
as
select * from view_inner_zj_devcount_sub5;





/* 建立第4小题的view */
drop table if exists view_inner_zj_devunreg_sub1;
create table view_inner_zj_devunreg_sub1
as
select left(devstate_base_devid, 4) as branch1_id, count(*) as devunreg
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 4) in (select branch1_id from branch1)
group by branch1_id;


drop table if exists view_inner_zj_devunreg_sub2;
create table view_inner_zj_devunreg_sub2
as
select left(devstate_base_devid, 2) as branch1_id, count(*) as devunreg
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 2) in (select branch1_id from branch1)
and left(devstate_base_devid, 4) not in (select branch1_id from view_inner_zj_devunreg_sub1)
group by branch1_id;


drop table if exists view_inner_zj_devunreg_sub3;
create table view_inner_zj_devunreg_sub3
as
select * from view_inner_zj_devunreg_sub1
union
select * from view_inner_zj_devunreg_sub2;


drop view if exists view_inner_zj_devunreg;
create view view_inner_zj_devunreg
as
select branch1_id as bid1, branch1_name as name, devunreg
from branch1 natural join view_inner_zj_devunreg_sub3
order by bid1;




/* 建立第5小题的view */
drop table if exists view_inner_zj_devdup_sub1;
create table view_inner_zj_devdup_sub1
as
select left(devstate_base_devid, 4) as branch1_id, count(*) as devdup
from devstate_base 
where left(devstate_base_devid, 4) in (select branch1_id from branch1)
and length(devstate_base_devno) > 1
group by branch1_id;

drop table if exists view_inner_zj_devdup_sub2;
create table view_inner_zj_devdup_sub2
as
select left(devstate_base_devid, 2) as branch1_id, count(*) as devdup
from devstate_base 
where left(devstate_base_devid, 2) in (select branch1_id from branch1)
and left(devstate_base_devid, 4) not in (select branch1_id from branch1)
and length(devstate_base_devno) > 1
group by branch1_id;

drop table if exists view_inner_zj_devdup_sub3;
create table view_inner_zj_devdup_sub3
as
select * from view_inner_zj_devdup_sub1
union
select * from view_inner_zj_devdup_sub2;

drop view if exists view_inner_zj_devdup;
create view view_inner_zj_devdup
as
select branch1_id as bid1, branch1_name as name, devdup
from branch1 natural join view_inner_zj_devdup_sub3
order by bid1;











/* 建立第6小题的view */
drop view if exists view_devorg_zj;
create view view_devorg_zj
as
select bid1, name, ifnull(bid2num, 0) as bid2num, ifnull(devnum, 0) as devnum, 
ifnull(devcount, 0) as devcount, ifnull(devunreg, 0) as devunreg, ifnull(devdup, 0) as devdup
from(select * from(select * from(select * from(select *
from view_inner_zj_bid2num left join view_inner_zj_devnum
using(bid1, name)) as sub1
left join view_inner_zj_devcount
using(bid1, name)) as sub2
left join view_inner_zj_devunreg
using(bid1, name)) as sub3
left join view_inner_zj_devdup
using(bid1, name)) as sub4
order by bid1;


/*drop view if exists view_devorg_zj;
create view view_devorg_zj
as
select *
from ((((view_inner_zj_bid2num as a left join view_inner_zj_devnum as b using(bid1,name)) as sub1
left join view_inner_zj_devcount as c using(bid1,name)) as sub2
left join view_inner_zj_devunreg as d using(bid1,name)) as sub3
left join view_inner_zj_devdup as e using(bid1,name)) as sub4;
*/




/* 执行第1-6小题视图的验证 */
select * from view_inner_zj_bid2num;
select * from view_inner_zj_devnum;
select * from view_inner_zj_devcount;
select * from view_inner_zj_devunreg;
select * from view_inner_zj_devdup;
select * from view_devorg_zj;

/* 删除你建立的临时表、视图、过程、函数等 */
drop view if exists view_inner_zj_bid2num;
drop view if exists view_inner_zj_devnum;
drop view if exists view_inner_zj_devcount_sub1;
drop view if exists view_inner_zj_devcount_sub2;
drop view if exists view_inner_zj_devcount_sub3;
drop view if exists view_inner_zj_devcount_sub4;
drop table if exists view_inner_zj_devcount_sub5;
drop view if exists view_inner_zj_devcount;
drop view if exists view_inner_zj_devunreg;
drop table if exists view_inner_zj_devunreg_sub1;
drop table if exists view_inner_zj_devunreg_sub2;
drop table if exists view_inner_zj_devunreg_sub3;
drop view if exists view_inner_zj_devdup;
drop table if exists view_inner_zj_devdup_sub1;
drop table if exists view_inner_zj_devdup_sub2;
drop table if exists view_inner_zj_devdup_sub3;
drop view if exists view_devorg_zj;

/* 再次查看当前数据库中的表，应该仅有6张表，如果自己的视图、过程、函数等未删除干净则不得分 */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";
