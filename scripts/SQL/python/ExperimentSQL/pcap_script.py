import MySQLdb




def insertPcapSQL(cursor, expID, type):
    cursor.callproc('pcap_insert', (expID, type))
    res = cursor.fetchall()
    pcapID = res[0][0]
    return pcapID






if __name__ == "__main__":
    insertPcapSQL()
