#! /usr/bin/env python

import struct
from socket import ntohs

import dpkt
import pandas as pd
from sqlalchemy import create_engine
import numpy as np
from ggplot import *


def get_sql_query(query):
    # query the database
    engine = create_engine('mysql://ucla_triton@198.188.2.10/testdb')

    # store the query in a data frame
    with engine.connect() as conn, conn.begin():
        df = pd.read_sql_query(query, conn, parse_dates=['test_date', 'my_date', 'my_time'])
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
    drops = drop_by_column(df['id_real_pcap'])
    print len(drops)
    print len(df.index)
    res = pd.DataFrame({'drop_id': pd.Series(drops, index=df.index)})
    mdf = pd.merge(df, res, left_index=True, right_index=True)

    print mdf
    print type(mdf)

    mdf.to_csv("spq_drop_data.csv")


def plot_loss_results():
    # plot the dataframe
    df = pd.read_csv('drop_data.csv')

    plot = ggplot(aes(x='num_packets', y='drop_id', color='packet_size'), data=df)
    print plot + geom_point()


def save_query():
    df = get_sql_query()
    df.to_csv("param_tests.csv")
    return df


def PoliceParamsQuery():
    PoliceParamsSQL = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip))
    JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Policing Parameters Test V1' && success = 1;
    """

    return get_sql_query(PoliceParamsSQL)


def ShapingParametersQuery():
    ShapingParametersSQL = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip))
    JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Shaping Parameters Test V1' && success = 1;
    """

    return get_sql_query(ShapingParametersSQL)


def BaselineFinalQuery():
    BaselineFinalSQL = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip))
    JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Baseline Final V1' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(BaselineFinalSQL)


def ShapingFinalQuery():
    ShapingFinalSQL = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip))
    JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Shaping Final V1' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(ShapingFinalSQL)


def PolicingFinalQuery():
    policingFinal = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip))
    JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Policing Final V1' && success = 1 && num_packets = 5000;
    """
    return get_sql_query(policingFinal)


def SpqGoodIpQuery():
    SpqGoodIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM (Metadata JOIN good_ip USING (host_ip)) JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'SPQ Disc V2' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(SpqGoodIPs)


def SpqAllIpQuery():
    SpqAllIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM Metadata JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'SPQ Disc V2' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(SpqAllIPs)


def ShapingGoodIpQuery():
    ShapingGoodIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM (Metadata JOIN good_ip USING (host_ip)) JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Shaping--Co-Op -- discrimination' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(ShapingGoodIPs)


def ShapingAllIpQuery():
    ShapingAllIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM Metadata JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Shaping--Co-Op -- discrimination' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(ShapingAllIPs)


def PolicingAllIpQuery():
    policingAllIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM Metadata JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Policing--Co-Op--discrimination' && success = 1 && num_packets = 5000;
    """
    return get_sql_query(policingAllIPs)


def PolicingGoodIpQuery():
    PolicingGoodIps = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM (Metadata JOIN good_ip USING (host_ip)) JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Policing--Co-Op--discrimination' && success = 1 && num_packets = 5000;
    """
    return get_sql_query(PolicingGoodIps)


def CompressionAllIpQuery():
    compressionAllIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM Metadata JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Compression-Co-Op V3 -- discrimination' && success = 1 && num_packets = 5000;
    """

    return get_sql_query(compressionAllIPs)


def CompressionGoodIpQuery():
    compressionGoodIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip USING (host_ip)) JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Compression-Co-Op V3 -- discrimination' && success = 1 &&
      num_packets = 5000 && packet_size = 1024 && NOT(host_ip = '192.16.125.12') && discrimination_losses_str - BaseResults.base_losses_str >50 &&
      NOT(host_ip = '130.195.4.68') && NOT(host_ip = '131.179.150.72') ;
    """

    return get_sql_query(compressionGoodIPs)


def SingleSidedCompressionQuery():
    query = """
    SELECT  * ,
    discrimination_losses_str - base_losses_str AS delta_losses,
    discrimination_time - base_time AS delta_time,
    DATE(test_date) AS my_date,
    TIME(test_date) AS my_time
    FROM (Metadata JOIN good_ip ON dest_ip = good_ip.name) JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'SingleSided Compression Discrimination' && success = 1 && num_packets = 5000;
    """
    return get_sql_query(query)


def old_police_query():
    policingAllIPs = """
    SELECT  * , discrimination_losses_str - base_losses_str AS delta, DATE(test_date) AS my_date, TIME(test_date) AS my_time FROM Metadata JOIN
    (
        CommonData JOIN
        (
            BaseResults JOIN
            (
                DiscriminationResults JOIN pcap_data USING (id_Experiments)
            )
            USING (id_Experiments)
        )
        USING (id_Experiments)
    )
    USING (id_Experiments)
    WHERE test_name = 'Policing--Co-Op' && success = 1 && num_packets = 5000;
    """
    return get_sql_query(policingAllIPs)


def plot_aggregate_results(filename):
    df = pd.read_csv(filename)
    # lng = pd.melt(df, id_vars=['my_time'])
    plot = ggplot(aes(x='junk_interval', y='delta', color="host_ip"), data=df)
    print plot + \
          geom_point() + \
          xlab("Separation Packet Train Length") + \
          ylab("Discrimination - Base Losses") + \
          ggtitle("Effect of Separation Packet Train Length on Relative Loss(5MB Total Transfer)")


def plot_aggregate_matrix(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    # df['date'] = df['test_date'].apply(lambda x: x.strftime('%d%m%Y'))
    # df['time'] = df['test_date'].apply(lambda x: x.strftime('%H:%M:%S'))
    # df['time'] = pd.to_datetime(df['time'])
    # print df['time']
    # xmin = df['time'].index.min()
    # xmax = df.loc(df['time'].index.max(), 'time')
    # xmin=pd.to_datetime('2016-04-29')
    # xmax=pd.to_datetime('2016-04-30')
    # df['percent'] = df['base_losses_str']/df['discrimination_losses_str']

    plot = ggplot(aes(x='test_date', y='delta', color="host_ip"), data=df)
    print plot + \
          geom_line(size=5) + \
          xlab(xlabel) + \
          ylab(ylabel) + \
          ggtitle(plot_title + ' Losses')  # + \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def plot_avg_losses(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    q = pd.Series.unique(df['host_ip'])
    avgs = {}
    for ip in q:
        r = df.query('host_ip == @ip')
        avgs[ip] = pd.Series.mean(r['delta'])

    # p = {'host_ip':pd.Series( data=avgs.keys()), 'avg':pd.Series(avgs.values())}

    # s = pd.DataFrame(p)

    s = df.groupby(['host_ip', 'country'])['delta'].mean().reset_index()

    print s

    plot = ggplot(aes(x='host_ip', y='delta', fill='country'), data=s)
    print plot + \
          geom_histogram(stat='bar') + \
          scale_x_discrete("host_ip") + \
          xlab('IP Address') + \
          ylab('Delta Losses')   + \
          ggtitle(plot_title + ' Average Losses') #+ \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def plot_avg_delay(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    df['delta_t'] = df['discrimination_time'] - df['base_time']
    q = df.query('delta_t > -4000 and delta_t < 10000')

    """
    q = pd.Series.unique(df['host_ip'])

    avgs = {}
    for ip in q:
        r = df.query('host_ip == @ip')
        avgs[ip] = pd.Series.mean(r['delta'])
    """
    # p = {'host_ip':pd.Series( data=avgs.keys()), 'avg':pd.Series(avgs.values())}

    # s = pd.DataFrame(p)

    s = q.groupby(['host_ip', 'country'])['delta_t'].mean().reset_index()

    print s

    plot = ggplot(aes(x='host_ip', y='delta_t', fill='country'), data=s)
    print plot + \
          geom_histogram(stat='bar') + \
          scale_x_discrete("host_ip") + \
          xlab('IP Address') + \
          ylab('Delta Time (ms)')   + \
          ggtitle(plot_title + ' Average Delay') #+ \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def plot_avg_delay_ss(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    df['delta_t'] = df['discrimination_time'] - df['base_time']
    q = df.query('delta_t > -4000 and delta_t < 10000')

    """
    q = pd.Series.unique(df['host_ip'])

    avgs = {}
    for ip in q:
        r = df.query('host_ip == @ip')
        avgs[ip] = pd.Series.mean(r['delta'])
    """
    # p = {'host_ip':pd.Series( data=avgs.keys()), 'avg':pd.Series(avgs.values())}

    # s = pd.DataFrame(p)

    s = q.groupby(['dest_ip', 'country'])['delta_t'].mean().reset_index()

    print s

    plot = ggplot(aes(x='dest_ip', y='delta_t', fill='country'), data=s)
    print plot + \
          geom_histogram(stat='bar') + \
          scale_x_discrete("host_ip") + \
          xlab('Time of Day') + \
          ylab('Delta Time (Seconds)')   + \
          ggtitle(plot_title + ' Average Delay') #+ \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def plot_delay_confidence_ss(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    df['delta_t'] = df['discrimination_time'] - df['base_time']
    q = df.query('delta_t > -4000 and delta_t < 10000')

    """
    q = pd.Series.unique(df['host_ip'])

    avgs = {}
    for ip in q:
        r = df.query('host_ip == @ip')
        avgs[ip] = pd.Series.mean(r['delta'])
    """
    # p = {'host_ip':pd.Series( data=avgs.keys()), 'avg':pd.Series(avgs.values())}

    # s = pd.DataFrame(p)



    s = q.groupby(['dest_ip', 'country'])

    z = s['delta_t'].agg([np.mean, np.min, np.max]).reset_index()


    plot = ggplot(aes(x='dest_ip', y='mean', ymin='amin', ymax='amax', fill='country' ), data=z)
    print plot + \
          geom_pointrange(width =100) + \
          scale_x_discrete("dest_ip") + \
          xlab('Time of Day') + \
          ylab('Delta Time (Seconds)')   + \
          ggtitle(plot_title + ' Average Delay') #+ \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def print_base_losses(filename):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])
    # q = pd.Series.unique(df['host_ip'])
    #q = df.query('host_ip == "165.242.90.129" and delta < 500')
    q = df.query('host_ip == "129.63.159.102" and delta < 500')
    print q['host_ip']
    print q['base_losses_str']
    print q['discrimination_losses_str']
    print q['discrimination_time'] -q['base_time']
    print q['id_pcap_data']
    print q['id.4']


def plot_single_sided_compression_aggregate(filename):
    df = pd.read_csv(filename)

    df['test_date'] = pd.to_datetime(df['test_date'])
    # df['my_time'] = pd.to_datetime(df['my_time'])
    df['time_diff'] = df['discrimination_time'] - df['base_time']
    # df['date'] = df['test_date'].apply(lambda x: x.strftime('%d%m%Y'))
    # df['time'] = df['test_date'].apply(lambda x: x.strftime('%H:%M:%S'))
    # df['time'] = pd.to_datetime(df['time'])
    # print df['time']
    # xmin = df['time'].index.min()
    # xmax = df.loc(df['time'].index.max(), 'time')
    # xmin=pd.to_datetime('2016-04-29')
    # xmax=pd.to_datetime('2016-04-30')

    # xlabel = "Separation Packet Train Length"
    xlabel = "Time of Day"
    ylabel = "Delta Time [Seconds]"
    # plot_title = "Shaping Detection"
    # plot_title = "Policing Detection"
    plot_title = "Compression Detection"

    # for time in  df['test_date']:
    #    print time.hour

    # q = pd.melt(df,id_vars='test_date')



    plot = ggplot(aes(x='test_date', y='time_diff', color="dest_ip"), data=df)
    print plot + \
          geom_point(aes(size=50)) + \
          xlab(xlabel) + \
          ylab(ylabel) + \
          ggtitle(plot_title)  # + \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')

    print plot + \
          geom_line(size=5) + \
          xlab(xlabel) + \
          ylab(ylabel) + \
          ggtitle(plot_title)  # + \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def plot_old_policing():
    filename = 'policing_old.csv'
    # df = old_police_query()
    # df.to_csv(filename)
    df = pd.read_csv(filename)
    q = df.query('base_losses_str == 0 and discrimination_losses_str < 4800 and discrimination_losses_str > 100')

    filename = 'policing_old_processed.csv'
    q.to_csv(filename)


def plot_agregate_delay(filename, xlabel, ylabel, plot_title):
    df = pd.read_csv(filename)
    df['test_date'] = pd.to_datetime(df['test_date'])

    df['delta_t'] = df['discrimination_time'] - df['base_time']
    q = df.query('delta_t > -4000 and delta_t < 10000')

    # df['delta_t'] = pd.Series.apply(df['delta_t'],abs)

    # q = df.groupby(['host_ip','country'])
    # print q

    plot = ggplot(aes(x='test_date', y='delta_t', color="host_ip"), data=q)
    print plot + \
          geom_line(size=5) + \
          xlab(xlabel) + \
          ylab('Delta Delay (ms)') + \
          ggtitle(plot_title + ' Delay')  # + \
    # facet_wrap("num_packets" ) + \
    # scale_x_date(labels='%H:%M:%S')


def get_query_dict():
    """
    queries = {"ShapingAggregatesGoodIp.csv": ShapingGoodIpQuery,
               # "ShapingAggregatesAllIp.csv" : ShapingAllIpQuery,
               "SpqAggregatesGoodIp.csv": SpqGoodIpQuery,
               # "SpqAggregatesAllIp.csv" : SpqAllIpQuery,
               # "PolicingAggregatesAllIPs.csv" : PolicingAllIpQuery,
               "PolicingAggregatesGoodIPs.csv": PolicingGoodIpQuery,
               # "CompressionAggregatesAllIPs.csv" : CompressionAllIpQuery,
               "CompressionAggregatesGoodIPs.csv": CompressionGoodIpQuery,
               #"SingleSidedCompressionAggregates.csv": SingleSidedCompressionQuery,
               "PolicingFinal.csv": PolicingFinalQuery,
               "ShapingFinal.csv": ShapingFinalQuery}"""
    #queries = {"BaselineFinal.csv":BaselineFinalQuery}
    queries = {"ShapingParameters.csv"  : ShapingParametersQuery,
               "PolicingParameters.csv" : PoliceParamsQuery}
    return queries


def get_queries():
    queries = get_query_dict()

    for key in queries:
        df = queries[key]()
        df.to_csv(key)


def process_data():
    q = get_query_dict()

    # xlabel = "Separation Packet Train Length"
    xlabel = "Time of Day"
    ylabel = "Delta Losses"
    # plot_title = "Shaping Detection"
    # plot_title = "Policing Detection"
    # plot_title = "Compression Detection"

    for k in q:
        print k
        if k == "SingleSidedCompressionAggregates.csv":
            continue
        plot_aggregate_matrix(k, xlabel, ylabel, k)


def process_data_general(func):
    q = get_query_dict()

    # xlabel = "Separation Packet Train Length"
    xlabel = "Time of Day"
    # ylabel = "Delta Losses"
    ylabel = "Delta Losses"
    # plot_title = "Shaping Detection"
    # plot_title = "Policing Detection"
    plot_title = "Compression Detection"

    for k in q:
        print k
        func(k, xlabel, ylabel, k)


def main():
    # filename = "ShapingAggregatesGoodIp.csv"
    # filename = "PolicingAggregatesAllIPs.csv"
    # filename = "PolicingAggregatesGoodIPs.csv"
    # filename = "CompressionAggregatesAllIPs.csv"
    # filename = "CompressionAggregatesGoodIPs.csv"
    # filename = "SingleSidedCompressionAggregates.csv"
    # df =SingleSidedCompressionQuery()
    # df = get_sql_query()
    # df = ShapingGoodIpQuery()
    # df.to_csv(filename)
    # pcap_files = get_pcaps("param_tests.csv")
    # process_csv("spq_param_tests.csv")
    # plot_aggregate_results()
    # plot_aggregate_matrix(filename)
    # plot_single_sided_compression_aggregate(filename)

    #get_queries()

    # filename = "SpqAggregatesGoodIp.csv"
    # df = SpqGoodIpQuery()
    # df.to_csv(filename)

    #process_data()
    #process_data_general(plot_avg_losses)
    #process_data_general(plot_agregate_delay)
    #process_data_general(plot_avg_delay)

    # print_base_losses("ShapingFinal.csv")
    #print_base_losses("SpqAggregatesGoodIp.csv")

    #plot_delay_confidence_ss("SingleSidedCompressionAggregates.csv","Time of Day", "Delta Time [Seconds]",                   "Non-cooperative Compression Detection")


if __name__ == "__main__":
    main()
