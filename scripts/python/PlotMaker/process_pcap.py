#! /usr/bin/env python

import struct
from socket import ntohs

import dpkt
import pandas as pd
from sqlalchemy import create_engine
from ggplot import *

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
    where test_name = 'SPQ Junk Parameters';
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


def drop_by_column(column, num_packets):
    l = []
    for item, n in zip(column, num_packets):
        l.append(add_drop_to_row(item, n))
    return l


def add_drop_to_row(pcap_id, num_packets):
    name = str(pcap_id) + '.pcap'
    res = find_first_drop(name, num_packets)
    return res[1] if res[0] else 0


def process_csv(filename):
    df = pd.read_csv(filename)
    columnName = 'id_real_pcap'
    #columnName = 'id_pcap_data'
    #print df['num_packets']
    drops = drop_by_column(df[columnName], df['num_packets'])
    print len(drops)
    print len(df.index)
    res = pd.DataFrame({'drop_id': pd.Series(drops, index=df.index)})
    mdf = pd.merge(df, res,left_index=True,right_index=True)

    print mdf
    print type(mdf)

    mdf.to_csv("policing_processed.csv")


def find_first_drop(pcap_file, num_packets):
    """ process a pcap file for plotting number of first dropped packet """
    # open the pcap file
    with open(pcap_file) as f:
        pcap = dpkt.pcap.Reader(f)
        i = 1
        # go through each packet and look for packet id
        for ts, buf in pcap:
            eth = dpkt.sll.SLL(buf)
            ip = eth.data
            udp = ip.data
            packet_id = ntohs(struct.unpack_from('H', udp.data)[0])

            if packet_id == 0 :
                continue

            #return the id of the first packet drop
            if i != packet_id :
            #if ((i-1)*3 +1) != packet_id and  udp.dport == 22223:
                return True, i
            i += 1
        #print num_packets
        if i < num_packets:
            return True, i
    return False, 0


def plot_loss_results(filename):
    # plot the dataframe
    df = pd.read_csv(filename)
    xlabel = "Number of Packets"

    ylabel = "ID of First Lost Packet"
    #ylabel = 'Delta Losses'

    print df['id_real_pcap'], df['num_packets'] , df['drop_id']


    plot = ggplot(aes(x='num_packets', y='drop_id', color='packet_size'), data = df) +\
        xlab(xlabel) +\
        ylab(ylabel) +\
        ggtitle(filename + " Paramter Test")
    print plot + geom_point(size=50)
    print plot + geom_line(size=5)
    print plot + facet_wrap('packet_size') + geom_line(size=4)


def save_query():
    df = get_sql_query()
    df.to_csv("param_tests.csv")
    return df


def main():
    #df = get_sql_query()
    #df.to_csv("spq_junk_param_tests.csv")
    #pcap_files = get_pcaps("param_tests.csv")
    #process_csv("spq_junk_param_tests.csv")
    #process_csv("spq_delta_param_tests.csv")
    #process_csv("spq_param_tests.csv")
    #process_csv("spqtest.csv")
    process_csv("PolicingParameters.csv")
    plot_loss_results("policing_processed.csv")


if __name__ == "__main__":
    main()
