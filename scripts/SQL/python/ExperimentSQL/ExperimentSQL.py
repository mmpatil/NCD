import subprocess
import MySQLdb


def startExperiment():
    """start the experiment and get the experimental id"""
    print "Starting Test...\n"
    db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")

    cursor = db.cursor()

    cursor.execute("INSERT INTO `Experiments` () VALUES () ")

    cursor.execute("SELECT LAST_INSERT_ID();")

    res = cursor.fetchall()

    db.commit()

    expID = res[0][0]

    print "Experiment ID: %d " % expID

    # start the measurment client passed in from commandline ... or maybe it will use config file...
    # subprocess.call(["detector",""])

    cursor.callproc('pcap_insert', (expID, "new poop"))

    # cursor.execute("select * from CommonData")
    # cursor.execute('SELECT @_pcap_insert_0, @_pcap_insert_1')

    res = cursor.fetchall()

    pcapID = res[0][0]

    print pcapID

    print "\n...Ending Test"


if __name__ == "__main__":
    startExperiment()
