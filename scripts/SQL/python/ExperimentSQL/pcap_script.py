#!/usr/bin/env python

import sys
import MySQLdb

from ExperimentSQL import getCfg, insertToSQL


def getPcapID(expID):
    hash = getCfg("sql.cfg")

    # connect to the database
    db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")
    # db = MySQLdb.connect(hash["host"], hash["user"], hash["passwd"], hash["db"])

    # obtain method to interact with DB
    cursor = db.cursor()
    print insertPcapSQL(db, cursor, expID, "new type")


def insertPcapSQL(db,cursor, expID, type):
    sql = "INSERT INTO `pcap_data` (`id_Experiments`,`Measurement Type`) VALUES('%s','%s');" % (expID, type)
    pcapID, success = insertToSQL(db, cursor, sql)

    return pcapID


if __name__ == "__main__":
    getPcapID(sys.argv[1])
