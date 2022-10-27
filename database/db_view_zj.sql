use yzmon;	/* ��yzmon����Ϊȱʡ���ݿ� */

/* �鿴��ǰ���ݿ��еı�Ӧ��ֻ��6�ű� */
show tables;
show function status where db = "yzmon";
show procedure status  where db = "yzmon";

/* ������1С���view���˴���Ϊdemo�������ҵʱɾ�������µļ��ɣ�*/
/* ע������ */
drop view if exists view_inner_zj_bid2num;	/* �����ͼ�Ѵ�������ɾ�� */
create view view_inner_zj_bid2num
as
select branch1_id as bid1, branch1_name as name, bid2num
from branch1 natural join (select branch2_branch1_id as branch1_id, COUNT(*) as bid2num
from branch2
group by branch1_id) as subx;




/* ������2С���view */
drop view if exists view_inner_zj_devnum;
create view view_inner_zj_devnum
as
select branch1_id as bid1, branch1_name as name, COUNT(*) as devnum
from branch1 natural join branch2 join devorg
where branch1_id = branch2_branch1_id and branch2_id = devorg_branch2_id
group by branch1_id, name;



/* ������3С���view */


/*��ʱ��ͼ1*/
drop view if exists view_inner_zj_devcount_sub1;
create view view_inner_zj_devcount_sub1
as
select distinct branch1_id as bid1, branch1_name as name, devorg_id
from branch1 natural join branch2 natural join devorg natural join devstate_base
where branch1_id = branch2_branch1_id and
branch2_id = devorg_branch2_id and
devorg.devorg_id = devstate_base.devstate_base_devid;

/*�Լƻ������е�������ʱ��ͼ2����4λ����*/
drop view if exists view_inner_zj_devcount_sub2;
create view view_inner_zj_devcount_sub2
as
select bid1
from view_inner_zj_devcount_sub1
where bid1 like '____';

/*��ͳ�Ƽƻ�������*/
drop view if exists view_inner_zj_devcount_sub3;
create view view_inner_zj_devcount_sub3
as
select bid1, name, count(*) as devcount
from view_inner_zj_devcount_sub1
where bid1 in (select bid1 from view_inner_zj_devcount_sub2) and bid1 = left(devorg_id, 4)
group by bid1, name;

/*��ͳ����λ������ѯ���ֱ�Ӽ�ȥ��*/


/*ĳʡ���мƻ������е�dev��������������*/
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

/*������������ͼ3*/
drop view if exists view_inner_zj_devcount;
create view view_inner_zj_devcount
as
select * from view_inner_zj_devcount_sub5;





/* ������4С���view */
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




/* ������5С���view */
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











/* ������6С���view */
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




/* ִ�е�1-6С����ͼ����֤ */
select * from view_inner_zj_bid2num;
select * from view_inner_zj_devnum;
select * from view_inner_zj_devcount;
select * from view_inner_zj_devunreg;
select * from view_inner_zj_devdup;
select * from view_devorg_zj;

/* ɾ���㽨������ʱ����ͼ�����̡������� */
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

/* �ٴβ鿴��ǰ���ݿ��еı�Ӧ�ý���6�ű�����Լ�����ͼ�����̡�������δɾ���ɾ��򲻵÷� */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";
