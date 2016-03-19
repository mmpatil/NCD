-- --------------------------------------------------------------------------------
-- Routine DDL
-- Note: comments before and after the routine body will not be stored by the server
-- --------------------------------------------------------------------------------
DELIMITER $$

CREATE DEFINER=`root`@`localhost` PROCEDURE `pcap_insert`(in expid integer,  in measurement_type varchar(64))
BEGIN
/*
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
    END;
*/
    start transaction;
        INSERT INTO `pcap_data` (`id`,`id_Experiments`,`Measurement Type`) VALUES
        ('','','');
    commit;
END