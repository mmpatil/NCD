#!/usr/bin/env python

import sys
import MySQLdb

from ExperimentSQL import getCfg


def getPcapID(expID):
    hash = getCfg("sql.cfg")

    # connect to the database
    # db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")
    db = MySQLdb.connect(hash["host"], hash["user"], hash["passwd"], hash["db"])

    # obtain method to interact with DB
    cursor = db.cursor()
    print insertPcapSQL(cursor, expID, "new type")



def insertPcapSQL(cursor, expID, type):
    cursor.callproc('pcap_insert', (expID, type))
    res = cursor.fetchall()
    pcapID = res[0][0]
    return pcapID






if __name__ == "__main__":
    getPcapID(sys.argv[1])
