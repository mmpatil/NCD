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

    hash = dict(item.split("=") for item in lines if not (item.strip().startswith("#") or item.isspace() or len(item)==0))

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
    #db = MySQLdb.connect(host="localhost", user="root", passwd="", db="testdb")
    db = MySQLdb.connect(hash["host"], hash["user"], hash["passwd"], hash["db"])

    # obtain method to interact with DB
    cursor = db.cursor()

    #start transaction -- do in a ty catch block so we can roll back on exceptions
    try:
        # add a new experiment
        cursor.execute("INSERT INTO `Experiments` () VALUES () ")

        # get the id back from the remote server
        #TODO: make this process a stored procedure
        cursor.execute("SELECT LAST_INSERT_ID();")

        # commit the
        db.commit()

    except:
        db.rollback()

    # get the ID back from DB
    res = cursor.fetchall()
    expID = res[0][0]

    print "Experiment ID: %d\n" % expID

    # start the measurement client -- passed in from commandline ... or maybe it will use config file...
    # subprocess.call(["detector", expID,...""])

    #add the pcap data -- should be in another script....
    cursor.callproc('pcap_insert', (expID, "new poop"))

    # cursor.execute("select * from CommonData")
    # cursor.execute('SELECT @_pcap_insert_0, @_pcap_insert_1')

    res = cursor.fetchall()

    pcapID = res[0][0]

    print pcapID

    print "\n...Ending Test"


def serverExperiment():
    pass



if __name__ == "__main__":
    clientExperiment()
