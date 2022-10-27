use score;

/* 查看当前数据库中的表，应该只有4张表 */
show tables;
show function status where db = "score";
show procedure status where db = "score";

/*****************************************************************
             proc_get_statscore
 *****************************************************************/


drop procedure if exists proc_get_statscore;
delimiter $$
CREATE PROCEDURE proc_get_statscore(IN in_score_id int)
BEGIN

select examtype_name as etype, exams_name as ename, (0 + cast(score_mark as char)) as score, count(*) as num
from exams natural join examtype natural join score
where exams.exams_id = score.score_id
and exams.exams_type = examtype.examtype_id
and exams.exams_id = `in_score_id`
group by score_mark
order by score desc;

END $$
delimiter ;

/*****************************************************************
             proc_get_range_num
 *****************************************************************/
drop procedure if exists proc_get_range_num;
delimiter $$
CREATE PROCEDURE proc_get_range_num(IN in_score_id int, IN is_score_type_total int)
BEGIN

    if is_score_type_total = 0 then
        select count((score_mark>=95 and score_mark<=100) or NULL) as A1,
        count((score_mark>=90 and score_mark<95) or NULL) as A2,
        count((score_mark>=85 and score_mark<90) or NULL) as B1,
        count((score_mark>=80 and score_mark<85) or NULL) as B2,
        count((score_mark>=70 and score_mark<80) or NULL) as C,
        count((score_mark>=60 and score_mark<70) or NULL) as D,
        count((score_mark>=0 and score_mark<60) or NULL) as E,
        count((score_mark<0 or score_mark>100) or NULL) as ERR
        from exams natural join score
        where exams.exams_id = score.score_id
        and exams.exams_id = `in_score_id`
        group by exams_id;
    else
        select count((score_mark>=295 and score_mark<=300) or NULL) as A1,
        count((score_mark>=290 and score_mark<295) or NULL) as A2,
        count((score_mark>=285 and score_mark<290) or NULL) as B1,
        count((score_mark>=280 and score_mark<285) or NULL) as B2,
        count((score_mark>=270 and score_mark<280) or NULL) as C,
        count((score_mark>=260 and score_mark<270) or NULL) as D,
        count((score_mark>=0 and score_mark<260) or NULL) as E,
        count((score_mark<0 or score_mark>300) or NULL) as ERR
        from exams natural join score
        where exams.exams_id = score.score_id
        and exams.exams_id = `in_score_id`
        group by exams_id;
    end if;

END $$
delimiter ;






/*****************************************************************
             proc_get_num_max_min_avg_score
 *****************************************************************/


drop procedure if exists proc_get_num_max_min_avg_score;
delimiter $$
CREATE PROCEDURE proc_get_num_max_min_avg_score(IN in_score_id int)
BEGIN

    select count(*) as num,
    max(score_mark) as max,
    min(score_mark) as min,
    cast(avg(score_mark) as decimal(5,2)) as avg
    from exams natural join score
    where exams.exams_id = score.score_id
    and exams.exams_id = `in_score_id`
    and score.score_mark is not null
    group by exams_id;

END $$
delimiter ;

/*****************************************************************
             proc_get_score
 *****************************************************************/



drop procedure if exists proc_get_score;
delimiter $$
CREATE PROCEDURE proc_get_score(IN in_exams_type char(2), in in_score_min_id int,
in in_score_max_id int, in in_user_no char(8), in in_sort char(4))
BEGIN

drop table if exists view_sub1;
create table view_sub1
as
select right(user_no, 2) as no, user_name as name, exams_date as edate,
examtype_name as etype, exams_name as ename, score_mark as score, exams_id, user_no
from exams natural join examtype natural join score natural join user
where exams.exams_id = score.score_id
and exams.exams_type = examtype.examtype_id
and score.score_stuno = user.user_no
and exams_id >= in_score_min_id and exams_id <= in_score_max_id
and (in_exams_type is null or in_exams_type = examtype_id)
order by exams_id desc, score desc;


drop table if exists view_sub12;
create table view_sub12
as
select a.*, if(score is null, null, rank() over (partition by a.exams_id order by a.score desc)) as rank
from view_sub1 as a
order by exams_id desc, score desc;

drop table if exists view_sub2;
create table view_sub2
as
select *
from view_sub12
where ((select user_level from user where user_no = in_user_no ) > 5 or in_user_no = user_no)
order by exams_id desc, score desc;



/*select a.*,count(b.score)+1 rank from view_sub1 a 
left join view_sub1 b on a.edate = b.edate and a.score < b.score
group by a.no, a.name, a.edate, a.examtype_id, a.etype, a.ename
order by a.edate,rank;*/

/*select * ,(select count(distinct score)+1 from view_sub1 B where B.edate = A.edate and B.score > A.score) as rank 
from view_sub1 A 
order by edate,rank;*/


if (select count(distinct exams_id) from view_sub2) = 1 then
    if in_sort = 'asc' then
        select no, name, edate, etype, ename, (0 + cast(score as char)) as score, rank
        from view_sub2
        order by rank is null asc, rank asc, no asc;
    else if in_sort = 'desc' then
        select no, name, edate, etype, ename, (0 + cast(score as char)) as score, rank
        from view_sub2 
        order by rank is null desc, rank desc, no asc;
    else
        select no, name, edate, etype, ename, (0 + cast(score as char)) as score, rank
        from view_sub2 
        order by no asc;
    end if; end if;
else 
    if in_sort is null then
        select no, name, edate, etype, ename, (0 + cast(score as char)) as score, rank
        from view_sub2 
        order by exams_id desc, rank is null asc, rank asc, no asc;
    else
        select no, name, edate, etype, ename, (0 + cast(score as char)) as score, rank
        from view_sub2 
        order by no asc, exams_id desc;
    end if;
end if;

drop table if exists view_sub1;
drop table if exists view_sub12;
drop table if exists view_sub2;

END $$
delimiter ;

/* 测试用例，生成与answer一样的输出文件的调用顺序，不准改动 */
call proc_get_statscore(62);
call proc_get_statscore(67);
call proc_get_statscore(75);
call proc_get_statscore(23);
call proc_get_statscore(27);
call proc_get_statscore(117);
call proc_get_statscore(56);
call proc_get_statscore(17);
call proc_get_statscore(145);
call proc_get_statscore(139);
call proc_get_statscore(105);
call proc_get_statscore(127);

call proc_get_range_num(63, 1);
call proc_get_range_num(67, 0);
call proc_get_range_num(75, 0);
call proc_get_range_num(23, 0);
call proc_get_range_num(30, 1);
call proc_get_range_num(117, 0);
call proc_get_range_num(56, 0);
call proc_get_range_num(17, 0);
call proc_get_range_num(145, 1);
call proc_get_range_num(139, 0);
call proc_get_range_num(108, 1);
call proc_get_range_num(127, 0);

call proc_get_num_max_min_avg_score(62);
call proc_get_num_max_min_avg_score(67);
call proc_get_num_max_min_avg_score(75);
call proc_get_num_max_min_avg_score(23);
call proc_get_num_max_min_avg_score(27);
call proc_get_num_max_min_avg_score(117);
call proc_get_num_max_min_avg_score(56);
call proc_get_num_max_min_avg_score(17);
call proc_get_num_max_min_avg_score(145);
call proc_get_num_max_min_avg_score(139);
call proc_get_num_max_min_avg_score(105);
call proc_get_num_max_min_avg_score(127);

call proc_get_score(22,   62, 62, "ht110704", "asc");
call proc_get_score(13,   67, 67, "ht110734", "desc");
call proc_get_score(11,   75, 75, "ht110714", NULL);
call proc_get_score(11,   23, 23, "ht110706", "asc");
call proc_get_score(21,   27, 27, "ht110718", "desc");
call proc_get_score(13,  117, 117, "ht110721", NULL);
call proc_get_score(12,   56, 56, "ht110730", "asc");
call proc_get_score(12,   17, 17, "ht110702", "desc");
call proc_get_score(24,  145, 145, "ht110719", NULL);
call proc_get_score(13,  139, 139, "ht110711", "asc");
call proc_get_score(23,  105, 105, "ht110729", "desc");
call proc_get_score(11,  127, 127, "ht110733", NULL);

call proc_get_score(11, 1, 145, "ht110701", "asc");
call proc_get_score(12, 1, 145, "ht110702", "desc");
call proc_get_score(13, 1, 145, "ht110703", NULL);
call proc_get_score(21, 1, 145, "ht110704", "asc");
call proc_get_score(22, 1, 145, "ht110705", "desc");
call proc_get_score(23, 1, 145, "ht110706", NULL);
call proc_get_score(24, 1, 145, "ht110707", "asc");
call proc_get_score(NULL, 1, 145, "ht110708", "desc");
call proc_get_score(11, 1, 145, "ht110709", NULL);
call proc_get_score(12, 1, 145, "ht110710", "asc");
call proc_get_score(13, 1, 145, "ht110711", "desc");
call proc_get_score(21, 1, 145, "ht110712", NULL);
call proc_get_score(22, 1, 145, "ht110713", "asc");
call proc_get_score(23, 1, 145, "ht110714", "desc");
call proc_get_score(24, 1, 145, "ht110715", NULL);
call proc_get_score(NULL, 1, 145, "ht110716", "asc");
call proc_get_score(11, 1, 145, "ht110717", "desc");
call proc_get_score(12, 1, 145, "ht110718", NULL);
call proc_get_score(13, 1, 145, "ht110719", "asc");
call proc_get_score(21, 1, 145, "ht110720", "desc");
call proc_get_score(22, 1, 145, "ht110721", NULL);
call proc_get_score(23, 1, 145, "ht110722", "asc");
call proc_get_score(24, 1, 145, "ht110723", "desc");
call proc_get_score(NULL, 1, 145, "ht110724", NULL);
call proc_get_score(11, 1, 145, "ht110725", "asc");
call proc_get_score(12, 1, 145, "ht110726", "desc");
call proc_get_score(13, 1, 145, "ht110727", NULL);
call proc_get_score(21, 1, 145, "ht110728", "asc");
call proc_get_score(22, 1, 145, "ht110729", "desc");
call proc_get_score(23, 1, 145, "ht110730", NULL);
call proc_get_score(24, 1, 145, "ht110731", "asc");
call proc_get_score(NULL, 1, 145, "ht110732", "desc");
call proc_get_score(11, 1, 145, "ht110733", NULL);
call proc_get_score(12, 1, 145, "ht110734", "asc");
call proc_get_score(13, 1, 145, "ht110735", "desc");
call proc_get_score(21, 1, 145, "ht110736", NULL);
call proc_get_score(22, 1, 145, "ht110737", NULL);

call proc_get_score(22,   62, 62, "teacher", "asc");
call proc_get_score(22,   62, 62, "teacher", "desc");
call proc_get_score(22,   62, 62, "teacher", NULL);
call proc_get_score(13,   67, 67, "teacher", "asc");
call proc_get_score(13,   67, 67, "teacher", "desc");
call proc_get_score(13,   67, 67, "teacher", NULL);
call proc_get_score(11,   75, 75, "teacher", "asc");
call proc_get_score(11,   75, 75, "teacher", "desc");
call proc_get_score(11,   75, 75, "teacher", NULL);
call proc_get_score(11,   23, 23, "teacher", "asc");
call proc_get_score(11,   23, 23, "teacher", "desc");
call proc_get_score(11,   23, 23, "teacher", NULL);
call proc_get_score(21,   27, 27, "teacher", "asc");
call proc_get_score(21,   27, 27, "teacher", "desc");
call proc_get_score(21,   27, 27, "teacher", NULL);
call proc_get_score(13,  117, 117, "teacher", "asc");
call proc_get_score(13,  117, 117, "teacher", "desc");
call proc_get_score(13,  117, 117, "teacher", NULL);
call proc_get_score(12,   56, 56, "teacher", "asc");
call proc_get_score(12,   56, 56, "teacher", "desc");
call proc_get_score(12,   56, 56, "teacher", NULL);
call proc_get_score(12,   17, 17, "teacher", "asc");
call proc_get_score(12,   17, 17, "teacher", "desc");
call proc_get_score(12,   17, 17, "teacher", NULL);
call proc_get_score(24,  145, 145, "teacher", "asc");
call proc_get_score(24,  145, 145, "teacher", "desc");
call proc_get_score(24,  145, 145, "teacher", NULL);
call proc_get_score(13,  139, 139, "teacher", "asc");
call proc_get_score(13,  139, 139, "teacher", "desc");
call proc_get_score(13,  139, 139, "teacher", NULL);
call proc_get_score(23,  105, 105, "teacher", "asc");
call proc_get_score(23,  105, 105, "teacher", "desc");
call proc_get_score(23,  105, 105, "teacher", NULL);
call proc_get_score(11,  127, 127, "teacher", "asc");
call proc_get_score(11,  127, 127, "teacher", "desc");
call proc_get_score(11,  127, 127, "teacher", NULL);

call proc_get_score(11,   1, 145, "teacher", "asc");
call proc_get_score(11,   1, 145, "teacher", "desc");
call proc_get_score(11,   1, 145, "teacher", NULL);
call proc_get_score(24,   1, 145, "teacher", "asc");
call proc_get_score(24,   1, 145, "teacher", "desc");
call proc_get_score(24,   1, 145, "teacher", NULL);
call proc_get_score(NULL, 1, 145, "teacher", "asc");
call proc_get_score(NULL, 1, 145, "teacher", NULL);

/* 删除你建立的临时表、视图、过程、函数等 */
drop procedure if exists proc_get_statscore;
drop procedure if exists proc_get_range_num;
drop procedure if exists proc_get_num_max_min_avg_score;
drop procedure if exists proc_get_score;

/* 再次查看当前数据库中的表，应该仅有4张表，如果自己的视图、过程、函数等未删除干净则不得分 */
show tables;
show function status where db = "score";
show procedure status where db = "score";

