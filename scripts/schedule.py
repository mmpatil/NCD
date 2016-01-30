from datetime import date
from dateutil.rrule import rrule, DAILY

class ClassName(object):
    """docstring for """
    def __init__(self, arg):
        super(, self).__init__()
        self.arg = arg
         ScheduleMaker(object):
    """docstring for ScheduleMaker"""
    def __init__(self, ip_file, start_date, end_date, period, command):
        super(ScheduleMaker, self).__init__()
        self.ip_file = ip_file
        self.start_date = start_date
        self.end_date = end_date
        self.period = period
        self.command = command

        #TODO read ip list from file
        self.target_list = []


    def create_schedule(self):
        for day in range(self.start_date, self.end_date):
            write_day(schedule_day())

    def schedule_day(self):
        day = {}
        periods = 24*60/self.period
        for i in range(periods):
            schedule = random.sample(xrange(0*i,self.period*i, self.interval), self.target_list.len)
            day.update(dict(zip(schedule, self.target_list)))

        return day

    def write_file(self, day, date):
        pass
