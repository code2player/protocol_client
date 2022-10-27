use yzmon;

/* 查看当前数据库中的表，应该只有6张表 */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";

/* 如果过程已存在则删除 */
drop procedure if exists proc_get_tserver;


/*表5除了time以外的所有字段*/


/* 建立过程 */
delimiter $$
CREATE PROCEDURE proc_get_tserver(IN devid_in char(9), IN devno_in char(3))
BEGIN

    set @devid_in = devid_in;
    set @devno_in = devno_in;

    select concat(devstate_ttyinfo_devid, devstate_ttyinfo_devno) as devid, devstate_ttyinfo_ttyno as tno,
    concat('-共' , devstate_ttyinfo_scrnum , '屏') as sno, NULL as is_current, devstate_ttyinfo_type as protocol,
    concat("#" , devstate_ttyinfo_ttyip) as serverip, 
    concat(devstate_ttyinfo_ttyno , "(" , devstate_ttyinfo_readno , ")") as serverport, devstate_ttyinfo_state as state,
    NULL as type, NULL as tx_server, NULL as rx_server, NULL as tx_terminal, NULL as rx_terminal, NULL as ping_value
    from devstate_ttyinfo
    where devstate_ttyinfo_devid = @devid_in and devstate_ttyinfo_devno = @devno_in
    union
    select concat(devstate_scrinfo_devid, devstate_scrinfo_devno) as devid, devstate_scrinfo_ttyno as tno,
    devstate_scrinfo_scrno as sno, devstate_scrinfo_is_current as is_current, devstate_scrinfo_protocol as protocol,
    devstate_scrinfo_serverip as serverip, devstate_scrinfo_serverport as serverport, devstate_scrinfo_state as state,
    devstate_scrinfo_ttytype as type, devstate_scrinfo_tx_server as tx_server, devstate_scrinfo_rx_server as rx_server,
    devstate_scrinfo_tx_terminal as tx_terminal, devstate_scrinfo_rx_terminal as rx_terminal,
    concat(devstate_scrinfo_ping_min, "/", devstate_scrinfo_ping_avg, "/", devstate_scrinfo_ping_max) as ping_value
    from devstate_scrinfo
    where devstate_scrinfo_devid = @devid_in and devstate_scrinfo_devno = @devno_in
    order by tno, cast(sno as int)asc;




END $$
delimiter ;




/* 生成与answer一样的输出文件的调用顺序 */
call proc_get_tserver("110401235", "1");
call proc_get_tserver("120105029", "1");
call proc_get_tserver("130683003", "1");
call proc_get_tserver("142322006", "1");
call proc_get_tserver("150599007", "1");
call proc_get_tserver("152823007", "101");
call proc_get_tserver("210681027", "1");
call proc_get_tserver("210212021", "1");
call proc_get_tserver("220112005", "1");
call proc_get_tserver("231083009", "1");
call proc_get_tserver("320901029", "401");
call proc_get_tserver("321282023", "1");
call proc_get_tserver("330522016", "1");
call proc_get_tserver("330281040", "1");
call proc_get_tserver("341422017", "1");
call proc_get_tserver("350181017", "1");
call proc_get_tserver("350201053", "1");
call proc_get_tserver("360901006", "1");
call proc_get_tserver("371201016", "1");
call proc_get_tserver("371201016", "101");
call proc_get_tserver("370282515", "1");
call proc_get_tserver("411729108", "1");
call proc_get_tserver("420601041", "1");
call proc_get_tserver("431028006", "1");
call proc_get_tserver("442001019", "1");
call proc_get_tserver("440301001", "1");
call proc_get_tserver("440301001", "201");
call proc_get_tserver("440301001", "301");
call proc_get_tserver("440301001", "401");
call proc_get_tserver("440301001", "501");
call proc_get_tserver("440301001", "601");
call proc_get_tserver("440301001", "701");
call proc_get_tserver("440301001", "801");
call proc_get_tserver("450701014", "1");
call proc_get_tserver("460201005", "1");
call proc_get_tserver("500701037", "1");
call proc_get_tserver("511602019", "1");
call proc_get_tserver("520201011", "1");
call proc_get_tserver("530326003", "1");
call proc_get_tserver("540101020", "1");
call proc_get_tserver("610523002", "1");
call proc_get_tserver("620523005", "1");
call proc_get_tserver("630101055", "1");
call proc_get_tserver("640122004", "1");
call proc_get_tserver("652901002", "1");

/* 删除你建立的临时表、视图、过程、函数等 */
drop procedure if exists proc_get_tserver;



/* 再次查看当前数据库中的表，应该仅有6张表，如果自己的视图、过程、函数等未删除干净则不得分 */
show tables;
show function status where db = "yzmon";
show procedure status where db = "yzmon";

