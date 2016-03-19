DELIMITER //

CREATE PROCEDURE pcap_insert (in expid integer,  in measurement_type varchar(64))
BEGIN

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
    END;

    start transaction;
        INSERT INTO `pcap_data` (`id`,`id_Experiments`,`Measurement Type`) VALUES
        ('','','');
    commit;
END//
