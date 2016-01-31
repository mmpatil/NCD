import random, fileinput
from datetime import datetime, date, time
from dateutil.rrule import rrule, DAILY


class ScheduleMaker(object):
    """docstring for ScheduleMaker"""

    def __init__(self, ip_file, start_date:date, end_date: date, period:datetime, command):
        super(ScheduleMaker, self).__init__()
        self.ip_file = ip_file
        self.start_date = start_date
        self.end_date = end_date
        self.period = period
        self.command = command

        # TODO read ip list from file
        self.target_list = []
        self.output_file_name = "schedule.txt"

    def create_schedule(self):
        for day in range(self.start_date, self.end_date):
            self.write_day(self.schedule_day())

    def schedule_day(self, my_date: date):
        day = {}
        periods = 24 * 60 / self.period
        for i in range(periods):
            # sample get me a random sample of the minutes in an hour ... up to 60
            # instead of putting that in the dictionary, we should put an actual date time
            # we should also pass in the day's date, and remove logic to make times consitent for the whole day
            # datetime removes a great deal of that calculation

            # TODO: fix this function to use actual date objects correctly, will simplify much of the logic
            # and relieve a great deal of testing
            schedule = random.sample(xrange(0 * i, self.period * i, self.interval), len(self.target_list))
            scheduled_times = [ datetime.combine(my_date, time(minute = minutes)) for minutes in schedule ]





            day.update(dict(zip(schedule, self.target_list)))

        return day


    def write_day(self,day):
        with open(self.output_file_name) as outFile:
            outFile.write(day)
