import socket
import subprocess
import MySQLdb
import fileinput
import os


def getCfg(filename):
    """
    :param filename: (string) filename of the configuration file

    :rtype: dict
    """
    f = open(filename, 'r')

    lines = f.read().splitlines()

    hash = dict(
        item.split("=") for item in lines if not (item.strip().startswith("#") or item.isspace() or len(item) == 0))

    return hash


def clientExperiment():
    """start the experiment and get the experimental id"""

    # Create base experiment in DB



    pocfg = getCfg("/home/atlas/workspace/ncd/detector.cfg")

    print "program_options config:"
    print pocfg
    print

    hash = getCfg("sql.cfg")

    print hash
    # connect to the database
    # db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")
    db = MySQLdb.connect(hash["host"], hash["user"], hash["passwd"], hash["db"])

    # obtain method to interact with DB
    cursor = db.cursor()

    # start transaction -- do in a ty catch block so we can roll back on exceptions

    # get the ID back from DB

    (expID, success) = insertExperimentSQL(db, cursor)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()

    print "Experiment ID: %d\n" % expID

    (metadataID, success) = insertMetadataSQL(db, cursor, expID, pocfg, True)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()

    print "Metadata ID: %d\n" % metadataID

    # start the measurement client -- passed in from commandline ... or maybe it will use config file...
    subprocess.call(["./client", "--test_id_in=" + str(expID)])

    # add the pcap data -- should be in another script....
    # cursor.callproc('pcap_insert', (expID, "new poop"))

    # cursor.execute("select * from CommonData")
    # cursor.execute('SELECT @_pcap_insert_0, @_pcap_insert_1')



    pcapID = insertPcapSQL(cursor, expID, "Base")

    print pcapID

    print "\n...Ending Test"


def serverExperiment():
    pass


def insertCommonDataSQL(db, cursor, testID, opts):
    """
    :param db:
    :param cursor:
    :param testID:
    :param program_options:
    :return:
    """
    sql = "INSERT INTO `CommonData` (`id_Experiments`,`protocol`,`num_packets`,`num_tail`,`packet_size`) VALUES " \
          " ('%d','%d','%d','%d','%d');" %(testID, opts["trans_proto"], opts["num+packets"], opts["num_tail"], opts["data_length"])
    print sql

    try:
        cursor.execute(sql)
        cursor.execute("SELECT LAST_INSERT_ID();")
        db.commit()
        worked = True
    except:
        db.rollback()
        worked = False

    res = cursor.fetchall()
    print res

    return (res[0][0], worked)



def insertMetadataSQL(db, cursor, testID, options, success):
    """
    :param db:
    :param cursor:
    :param testID:
    :param options:
    :param success:
    :return:
    """

    """inserts test metadata into the given database using the provided testID"""

    worked = False
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        sql = "INSERT INTO `Metadata` (`id_Experiments`,`Project`,`test_name`,`test_date`,`command`,`host_ip`,`dest_ip`,`success`) VALUES ('%d','%s','%s',%s,'%s','%s','%s','%d');" % (
        testID, "Thesis", "Policing--Co-Op", "NOW()", "client --test_id_in=" + str(testID), ip, options["dest_ip"],
        success)
        print "sql =" + sql
        cursor.execute(sql)
        cursor.execute("SELECT LAST_INSERT_ID();")
        db.commit()
        worked = True
    except:
        db.rollback()
        worked = False
    finally:
        s.close()

    res = cursor.fetchall()
    print res

    return (res[0][0], worked)


"""
-- INSERT INTO `CommonData` (`id`,`id_Experiments`,`protocol`,`num_packets`,`num_tail`,`packet_size`) VALUES
-- ('','','','','','');
-- INSERT INTO `DiscriminationResults` (`id`,`id_Experiments`,`discrimination_time`,`discrimination_losses_str`,`disc_src_port`,`disc_dest_port`,`disc_tos`,`disc_id`,`disc_frag_off`,`disc_ttl`,`disc_filename`,`id_pcap_data`) VALUES
-- ('','','','','','','','','','','','');
-- INSERT INTO `BaseResults` (`id`,`id_Experiments`,`base_time`,`base_losses_str`,`base_src_port`,`base_dest_port`,`base_tos`,`base_id`,`base_ttl`,`base_frag_off`,`base_filename`,`id_pcap_data`) VALUES
-- ('','','','','','','','','','','','');
-- INSERT INTO `Experiments` (`id`) VALUES
-- ('');
-- INSERT INTO `Metadata` (`id`,`id_Experiments`,`Project`,`test_name`,`test_date`,`command`,`host_ip`,`dest_ip`,`success`) VALUES
-- ('','','','','','','','','');
-- INSERT INTO `pcap_data` (`id`,`id_Experiments`,`Measurement Type`) VALUES
-- ('','','');
"""


def insertPcapSQL(cursor, expID, type):
    cursor.callproc('pcap_insert', (expID, type))
    res = cursor.fetchall()
    pcapID = res[0][0]
    return pcapID


def insertExperimentSQL(db, cursor):
    success = False
    try:
        # add a new experiment
        cursor.execute("INSERT INTO `Experiments` () VALUES () ")

        # get the id back from the remote server
        # TODO: make this process a stored procedure
        cursor.execute("SELECT LAST_INSERT_ID();")

        # commit the
        db.commit()
        success = True
    except:
        db.rollback()
        success = False

    # get the ID back from DB
    res = cursor.fetchall()
    return (res[0][0], success)


def handleExperimentFailure():
    pass


if __name__ == "__main__":
    clientExperiment()
