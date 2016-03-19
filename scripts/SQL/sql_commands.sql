

insert into data (project   , test_name     , test_date , command   , host_ip       , dest_ip   , success   , high_time , low_time  , high_losses   , low_losses        , num_tail  , dest_port , src_port  , num_packets   , packet_size   , protocol) \
          Values ( 'NCD'    , 'Prototype'   , NOW()     , 'echo'    , '127.0.0.1'   , '8.8.8.8' , TRUE      , 25.001    , 2.275     , '1-1000'      , '7-500,502-1000'  , 20        , 33333     , 22222     , 1000          , 1024          , 'UDP' );


