import random, fileinput
from datetime import datetime, date, time, timedelta
from dateutil.rrule import rrule, DAILY


class ScheduleMaker(object):
    """docstring for ScheduleMaker"""

    def __init__(self, ip_file, start_date:date, end_date: date, period:timedelta, interval:timedelta, command):
        super(ScheduleMaker, self).__init__()
        self.ip_file = ip_file
        self.start_date = start_date
        self.end_date = end_date
        self.period = period
        self.interval = interval
        self.command = command

        # TODO read ip list from file
        self.target_list = []
        self.output_file_name = "schedule.txt"

    def create_schedule(self):
        for day in per_delta(self.start_date, self.end_date, timedelta(days = 1)):
            self.write_day(self.schedule_day(day))

    def schedule_day(self, my_date: date):
        day = {}
        d = datetime(my_date)
        for period in per_delta(d,d+timedelta(days=1), self.period):
            # sample get me a random sample of the minutes in an hour ... up to 60
            # instead of putting that in the dictionary, we should put an actual date time
            # we should also pass in the day's date, and remove logic to make times consistent for the whole day
            # datetime removes a great deal of that calculation

            # TODO: fix this function to use actual date objects correctly, will simplify much of the logic
            # and relieve a great deal of testing
            schedule = random.sample(per_delta(period, period + self. period, self.interval), len(self.target_list))
            #scheduled_times = [ datetime.combine(my_date, time(minute = minutes)) for minutes in schedule ]

            day.update(dict(zip(schedule, self.target_list)))

        return day


    def write_day(self,day):
        with open(self.output_file_name) as outFile:
            for key in day:
                outFile.write(key + "\t" + day[key] + "\n")


# taken from http://stackoverflow.com/questions/10688006/generate-a-list-of-datetimes-between-an-interval-in-python
def per_delta(start, end, delta):
    curr = start
    while curr < end:
        yield curr
        curr += delta