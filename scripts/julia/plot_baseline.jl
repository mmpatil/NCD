using MySQL

con = mysql_connect("198.188.2.10", "ucla_triton", "", "testdb")

query = """select * from Metadata join
	(
		CommonData join
		(
			BaseResults join DiscriminationResults using (id_Experiments)
		)
		using (id_Experiments)
	)
	using (id_Experiments);"""


today_query = """select * from Metadata join
	(
		CommonData join
		(
			BaseResults join DiscriminationResults using (id_Experiments)
		)
		using (id_Experiments)
	)
	using (id_Experiments) where Metadata.test_date > current_date();"""

drframe = mysql_execute(con, today_query)

mysql_disconnect(con)

plot(drframe,x = "id_Experiments", y = int(drframe[:discrimination_time]) - int(drframe[:base_time]))

#plot(drframe,x = "dest_ip", y="discrimination_losses_str" )


 base_losses = drframe[:base_losses_str]
 dis_losses = drframe[:discrimination_losses_str]
 plot(drframe,x = "id_Experiments" y = int(drframe[:discrimination_losses_str]) - int(drframe[:base_losses_str]))


plot(layer(drframe, x = "id_Experiments", y = "base_time", Theme(default_color=color("orange")), Geom.point), layer(drframe, x = "id_Experiments", y = "discrimination_time", Theme(default_color=color("purple")), Geom.point))

plot(drframe,x = "id_Experiments", y = int(drframe[:discrimination_time]) - int(drframe[:base_time]))
