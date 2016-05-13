#!/usr/bin/env python

import MySQLdb
import socket
import subprocess
import sys


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


def clientExperiment(args):
    """start the experiment and get the experimental id"""

    # Create base experiment in DB

    pocfg = getCfg("detector.cfg")
    for item in args[2:]:
        names = item.split('=')
        if names[0].strip('-') in pocfg.keys():
            pocfg[names[0].strip('-')] = names[1]

    hash = getCfg("sql.cfg")

    # connect to the database
    # db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")
    db = MySQLdb.connect(hash["host"], hash["user"], hash["passwd"], hash["db"])

    # obtain method to interact with DB
    cursor = db.cursor()

    # start transaction -- do in a ty catch block so we can roll back on exceptions
    # get the ID back from DB
    (expID, success) = insertExperimentSQL(db, cursor)

    if not success:
        print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()


    # start the measurement client -- passed in from commandline ... or maybe it will use config file...
    outfile = open("output.txt", 'w+')

    ret_code = subprocess.call(["./client", "--test_id_in=" + str(expID)] + args[2:], stdout=outfile)
    outfile.close()

    # track the success of the experiment
    # TODO: evaluate the possible return codes from timeout, and other commands to be sure success is correct
    exp_success = ret_code == 0

    (metadataID, success) = insertMetadataSQL(db, cursor, expID, args, pocfg, exp_success)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()

    (commonID, success) = insertCommonDataSQL(db, cursor, expID, pocfg)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()

    cursor.close()
    cursor= db.cursor()
    base, disc = processDetectionOutput("output.txt")
    baseID, success = insertBaseResultSQL(db, cursor, expID, base)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()

    discID, success = insertDiscriminationResultSQL(db, cursor, expID, disc)

    if not success:
        # print "Error creating a new experiment in the database... aborting"
        handleExperimentFailure()


def insertBaseResultSQL(db, cursor, testID, ret_hash):
    sql = "INSERT INTO `BaseResults` (`id_Experiments`,`base_time`,`base_losses_str`,`base_src_port`,`base_dest_port`,`base_tos`,`base_id`,`base_ttl`,`base_frag_off`,`base_filename`,`id_pcap_data`) VALUES " \
          "('%d','%s','%s','%s','%s','%s','%s','%s','%s',%s, '%s', '%s');" % (
              testID, ret_hash["milliseconds"], ret_hash["packets_lost"], ret_hash["sport"], ret_hash["dport"],
              ret_hash["tos"], ret_hash["id"], ret_hash["ttl"],
              ret_hash["frag_off"], ret_hash["filename"], ret_hash["pcap_id"], ret_hash["last_packet"])

    return insertToSQL(db, cursor, sql)


def insertDiscriminationResultSQL(db, cursor, testID, ret_hash):
    sql = "INSERT INTO `DiscriminationResults` (`id_Experiments`,`discrimination_time`,`discrimination_losses_str`,`disc_src_port`,`disc_dest_port`,`disc_tos`,`disc_id`,`disc_frag_off`,`disc_ttl`,`disc_filename`,`id_pcap_data`) VALUES " \
          "('%d','%s','%s','%s','%s','%s','%s','%s','%s',%s, '%s', '%s');" % (
              testID, ret_hash["milliseconds"], ret_hash["packets_lost"], ret_hash["sport"], ret_hash["dport"],
              ret_hash["tos"], ret_hash["id"], ret_hash["ttl"],
              ret_hash["frag_off"], ret_hash["filename"], ret_hash["pcap_id"], ret_hash['last_packet'])

    return insertToSQL(db, cursor, sql)


def serverExperiment():
    pass


def insertToSQL(db, cursor, insert_command):
    """ Inserts a new row into the selected database with the provided command
    :param db: (MySQLdb) The database to insert into
    :param cursor: database cursor
    :param insert_command: complete command to be executed
    :return: Tuple of the the id of the newly inserted item, and success of the db operation
    """
    try:
        cursor.execute(insert_command)
        cursor.execute("SELECT LAST_INSERT_ID();")
        db.commit()
        status = True
    except:
        db.rollback()
        status = False

    res = cursor.fetchall()

    return (res[0][0], status)


def insertCommonDataSQL(db, cursor, testID, opts):
    """
    :param db: (MySQLdb) The database to insert into
    :param cursor: database cursor
    :param testID: numeric experiment id
    :param opts: a dictionary of the program options used to create the SQL command
    :return: Tuple of the the id of the newly inserted item, and success of the db operation
    """
    sql = "INSERT INTO `CommonData` (`id_Experiments`,`protocol`,`num_packets`,`num_tail`,`packet_size`, `tail_wait`, `junk_interval`) VALUES " \
          " ('%d','%s','%s','%s','%s', '%s','%s');" % (
              testID, opts["trans_proto"], opts["num_packets"], opts["num_tail"], opts["data_length"], opts["tail_wait"], opts["junk_interval"])
    return insertToSQL(db, cursor, sql)


def insertMetadataSQL(db, cursor, testID, args, options, success):
    """inserts test metadata into the given database using the provided testID
    :param db: (MySQLdb) The database to insert into
    :param cursor: database cursor
    :param testID: numeric experiment id
    :param options: a dictionary of the program options used to create the SQL command
    :param success: (bool) the status of the experiment (True is success; False is failure)
    :return: Tuple of the the id of the newly inserted item, and success of the db operation
    """

    # use a temporary socket to obtain source IP from network interface
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    ip = s.getsockname()[0]
    sql = "INSERT INTO `Metadata` (`id_Experiments`,`Project`,`test_name`,`test_date`,`command`,`host_ip`,`dest_ip`,`success`) VALUES ('%d','%s','%s',%s,'%s','%s','%s','%d');" % (
        testID, "Thesis", args[1], "NOW()", "client --test_id_in=" + str(testID) +" " + " ".join( args[2:]), ip, options["dest_ip"],
        success)
    return insertToSQL(db, cursor, sql)


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


# def insertPcapSQL(cursor, expID, type):
    # cursor.callproc('pcap_insert', (expID, type))
    # res = cursor.fetchall()
    # pcapID = res[0][0]
    # return pcapID


def insertExperimentSQL(db, cursor):
    """
    :param db: the database to insert the experiment into
    :param cursor: cursor to navigate the db
    :return: Tuple of the the id of the newly inserted item, and success of the db operation
    """
    sql = "INSERT INTO `Experiments` () VALUES ();"

    return insertToSQL(db, cursor, sql)


def handleExperimentFailure():
    print "EXPERIMENT FAILED!!! --EXIT"
    raise


def processDetectionOutput(filename):
    columns = ["trans_proto", "src_ip", "dest_ip", "sport", "dport", "num_packets", "num_tail", "payload_size",
               "tail_wait", "filename","tos", "id", "ttl", "frag_off", "packets_lost", "milliseconds", "pcap_id",
               "last_packet"]

    f = open(filename)
    line = f.readline().split()
    #print line

    base = dict(zip(columns, line))
    base["filename"] = "\"" + base["filename"] +"\""

    line = f.readline().split()
    disc = dict(zip(columns, line))
    disc["filename"] = "\"" + disc["filename"] +"\""
    return base, disc


if __name__ == "__main__":
    clientExperiment(sys.argv)
