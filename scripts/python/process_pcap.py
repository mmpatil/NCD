#! /bin/env python

import sqlalchemy
from sqlalchemy import create_engine
import pandas as pd
import struct
import dpkt
from socket import ntohs


def get_sql_query():
    # query the database
    engine = create_engine('mysql://ucla_triton@198.188.2.10/testdb')
    query = """
    SELECT * from Metadata join
    (
        CommonData join
        (
            BaseResults join
            (
                DiscriminationResults join pcap_data using (id_Experiments)
            )
            using (id_Experiments)
        )
        using (id_Experiments)
    )
    using (id_Experiments)
    where test_name = 'Compression-Co-Op V3 -- parameter verification';
    """

    # store the query in a data frame
    with engine.connect() as conn, conn.begin():
        df = pd.read_sql_query(query, conn)
        return df


def process_dataframe(df):
    # select one of the results from the dataframe
    pass


def get_pcaps(df):
    pcaps = []
    for exp_id in df['id_Experiments']:
        res = df.query('id_Experiments == @exp_id')
        names = []
        for pcap_id in res['id_real_pcap']:
            name = str(pcap_id) + '.pcap'
            names.append(name)
        pcaps.append(names)
    return pcaps

def add_drops(df):
    for index, row in df.iterrows():
        df.iloc[index, 'drop'] = add_drop_to_row(row)
    return df

def drop_by_column(column):
    l = []
    for item in column:
        l.append(add_drop_to_row(item))
    return l



def add_drop_to_row(pcap_id):
    name = str(pcap_id) + '.pcap'
    res = find_first_drop(name)
    return res[1] if res[0] else 0


def process_csv(filename):
    df = pd.read_csv(filename)
    columnName = 'id_real_pcap'
    #print df[columnName]
    tester = df.query('id_real_pcap == 9151')
    tester['drop'] = drop_by_column(tester['id_real_pcap'])
    #print tester
    for i in tester.index:
        tester.iloc[i, 'drop'] = add_drop_to_row(tester.ix[i])



    #pcap_files =  get_pcaps(df)
    drops = []
    """for file_list in pcap_files:
        for pcap_name in file_pair:
            drops.append(find_first_drop(pcap_name))
    """



def find_first_drop(pcap_file):
    """ process a pcap file for plotting number of first dropped packet"""
    #open the pcap file
    with open(pcap_file) as f:
        pcap = dpkt.pcap.Reader(f)
        i = 1
        # go through each packet and look for packet id
        for ts, buf in pcap:
            eth = dpkt.sll.SLL(buf)
            ip = eth.data
            udp = ip.data
            packet_id = ntohs(struct.unpack_from('H', udp.data)[0])
            #return the id of the first packet drop
            if i != packet_id:
                return True, i
            i += 1
    return False, 0


def plot_loss_results():
    # plot the dataframe
    pass

def save_query():
    df = get_sql_query()
    df.to_csv("param_tests.csv")
    return df

def main():
    #pcap_files = get_pcaps("param_tests.csv")
    process_csv("param_tests.csv")


if __name__ == "__main__":
    main()
