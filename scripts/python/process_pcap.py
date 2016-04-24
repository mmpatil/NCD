#! /bin/env python

import sqlalchemy
from sqlalchemy import create_engine
import pandas as pd


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


def get_pcaps():
    pcaps = []
    df = pd.read_csv("param_tests.csv")
    for exp_id in df['id_Experiments']:
        res = df.query('id_Experiments == @exp_id')
        names = []
        for pcap_id in res['id_Experiments']:
            name = str(pcap_id) + '.pcap'
            names.append(name)

        pcaps.append(names)

    return pcaps


def find_first_drop(pcap_file):
    """ process a pcap file for plotting number of first dropped packet"""
    #open the pcap file

    # go through each packet and look for packet id

    #return the id of the first packet drop
    pass


def plot_loss_results():
    # plot the dataframe
    pass

def save_query():
    df = get_sql_query()
    df.to_csv("param_tests.csv")
    return df

def main():
    df = pd.read_csv("param_tests.csv")
    print df.columns.values

if __name__ == "__main__":
    main()
