use yzmon;

/* 查看当前数据库中的表，应该只有6张表 */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";

/* 建立各种临时表、视图、函数、过程等 */

/*1-3 col*/
drop view if exists view_sub1;
create view view_sub1
as
select branch1_id as bid1, branch2_id as bid2, branch2_name as bid2name
from branch1 natural join branch2
where branch1_id = branch2_branch1_id;

update view_sub1
set bid2name = (select branch1_name from branch1 where branch1_id = bid1)
where bid2name is NULL;


/*4 col*/
drop view if exists view_sub2;
create view view_sub2
as
select bid1, bid2, bid2name, COUNT(devorg_branch2_id) as devnum
from view_sub1 left join devorg
on bid2 = devorg_branch2_id
group by bid1, bid2, bid2name;

/*5 col*/
drop view if exists view_sub3;
create view view_sub3
as
select bid1, bid2, bid2name, devnum, devorg_id
from view_sub2 left join devorg
on bid2 = devorg_branch2_id;

drop view if exists view_sub4;
create view view_sub4
as
select distinct bid1, bid2, bid2name, devnum, devstate_base_devid
from view_sub3 left join devstate_base
on view_sub3.devorg_id = devstate_base.devstate_base_devid;

drop view if exists view_sub5;
create view view_sub5
as
select bid1, bid2, bid2name, devnum, count(distinct devstate_base_devid) as devcount
from view_sub4
group by bid1, bid2, bid2name, devnum;

/*6 col*/


drop view if exists view_sub61;
create view view_sub61
as
select left(devstate_base_devid, 4) as bid2, count(*) as devunregx
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 4) in (select bid2 from view_sub1)
and length(devstate_base_devno) > 1
group by bid2;


drop view if exists view_sub71;
create view view_sub71
as
select left(devstate_base_devid, 2) as bid2, count(*) as devunregx
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 2) in (select bid2 from view_sub1)
and left(devstate_base_devid, 4) not in (select bid2 from view_sub61)
and length(devstate_base_devno) > 1
group by bid2;


drop view if exists view_sub81;
create view view_sub81
as
select * from view_sub61
union
select * from view_sub71;



drop view if exists view_sub91;
create view view_sub91
as
select bid1, bid2, bid2name, devnum, devcount, ifnull(devunregx, 0) as devunregx
from view_sub5 left join view_sub81
using (bid2);


drop view if exists view_sub6;
create view view_sub6
as
select left(devstate_base_devid, 4) as bid2, count(*) as devunreg
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 4) in (select bid2 from view_sub1)
group by bid2;


drop view if exists view_sub7;
create view view_sub7
as
select left(devstate_base_devid, 2) as bid2, count(*) as devunreg
from devstate_base 
where devstate_base_devid not in (select devorg_id from devorg)
and left(devstate_base_devid, 2) in (select bid2 from view_sub1)
and left(devstate_base_devid, 4) not in (select bid2 from view_sub6)
group by bid2;


drop view if exists view_sub8;
create view view_sub8
as
select * from view_sub6
union
select * from view_sub7;



drop view if exists view_sub9;
create view view_sub9
as
select bid1, bid2, bid2name, devnum, devcount, devunregx, ifnull(devunreg, 0) as devunreg
from view_sub91 left join view_sub8
using (bid2);

/*col 7*/
drop view if exists view_sub10;
create view view_sub10
as
select devorg_branch2_id as bid2, count(*) as devdup
from devorg left join devstate_base on devorg.devorg_id = devstate_base.devstate_base_devid
where devorg_branch2_id in (select bid2 from view_sub1)
and length(devstate_base_devno) > 1
group by bid2;

drop view if exists view_sub11;
create view view_sub11
as
select devorg_branch2_id as bid2, count(*) as devdup
from devorg left join devstate_base on devorg.devorg_id = devstate_base.devstate_base_devid
where devorg_branch2_id in (select bid2 from view_sub1)
and length(devstate_base_devno) > 1
group by bid2;

drop view if exists view_sub12;
create view view_sub12
as
select * from view_sub10
union
select * from view_sub11;

drop view if exists view_sub13;
create view view_sub13
as
select bid1, bid2, bid2name, devnum, devcount, devunreg, (ifnull(devdup, 0) + devunregx) as devdup
from view_sub9 left join view_sub12
using (bid2)
order by bid1, bid2;


/* 如果过程已存在则删除 */
drop procedure if exists proc_get_devorg_b1s;

/* 建立过程 */
delimiter $$
CREATE PROCEDURE proc_get_devorg_b1s(IN bidin varchar(50))
BEGIN
    if bidin is null then
        select * from view_sub13;
    else
        select * from view_sub13 where bid1 = bidin;
    end if;
END $$
delimiter ;



/* 生成与answer一样的输出文件的调用顺序，不要改动 */
call proc_get_devorg_b1s(NULL);
call proc_get_devorg_b1s("11");
call proc_get_devorg_b1s("12");
call proc_get_devorg_b1s("13");
call proc_get_devorg_b1s("14");
call proc_get_devorg_b1s("15");
call proc_get_devorg_b1s("21");
call proc_get_devorg_b1s("2102");
call proc_get_devorg_b1s("22");
call proc_get_devorg_b1s("23");
call proc_get_devorg_b1s("31");
call proc_get_devorg_b1s("32");
call proc_get_devorg_b1s("33");
call proc_get_devorg_b1s("3302");
call proc_get_devorg_b1s("34");
call proc_get_devorg_b1s("35");
call proc_get_devorg_b1s("3502");
call proc_get_devorg_b1s("36");
call proc_get_devorg_b1s("37");
call proc_get_devorg_b1s("3702");
call proc_get_devorg_b1s("41");
call proc_get_devorg_b1s("42");
call proc_get_devorg_b1s("43");
call proc_get_devorg_b1s("44");
call proc_get_devorg_b1s("4403");
call proc_get_devorg_b1s("45");
call proc_get_devorg_b1s("46");
call proc_get_devorg_b1s("50");
call proc_get_devorg_b1s("51");
call proc_get_devorg_b1s("52");
call proc_get_devorg_b1s("53");
call proc_get_devorg_b1s("54");
call proc_get_devorg_b1s("61");
call proc_get_devorg_b1s("62");
call proc_get_devorg_b1s("63");
call proc_get_devorg_b1s("64");
call proc_get_devorg_b1s("65");

/* 删除你建立的临时表、视图、过程、函数等 */
drop procedure if exists proc_get_devorg_b1s;
drop view if exists view_sub1;
drop view if exists view_sub2;
drop view if exists view_sub3;
drop view if exists view_sub4;
drop view if exists view_sub5;
drop view if exists view_sub6;
drop view if exists view_sub7;
drop view if exists view_sub8;
drop view if exists view_sub9;
drop view if exists view_sub61;
drop view if exists view_sub71;
drop view if exists view_sub81;
drop view if exists view_sub91;
drop view if exists view_sub10;
drop view if exists view_sub11;
drop view if exists view_sub12;
drop view if exists view_sub13;

/* 再次查看当前数据库中的表，应该仅有6张表，如果自己的视图、过程、函数等未删除干净则不得分 */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";
