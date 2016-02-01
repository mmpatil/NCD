import random, fileinput
from datetime import datetime, date, time, timedelta


class ScheduleMaker(object):
    """Creates schedules for use in automation tasks.

    The schedules created should have no conflicts
    between tasks, i.e. tasks only appear once in a
    Period and only one task may appear in an Interval.

    Periods denote how often a task should be scheduled,
    and limit the number of tasks that can be executed
    for a particular target IP address.
    Intervals denote the maximum running time for a task,
    and also limit the number of target IP addresses that
    can be used.
    """

    def __init__(self, ip_file: str, start_date: date, end_date: date, period: timedelta, interval: timedelta, command: str):
        """ initializes the class
        :param ip_file: (string) file name of a fil that contains a list of remote host IP addresses, one per line
        :param start_date: (date) provides the date when scheduling starts
        :param end_date: (date) provides the date when scheduling ends
        :param period: (timedelta) The interval at which a test should be run again for same IP
        :param interval: (timedelta) The maximum length of a test, limits how many tests per Period
        :param command: (string) the command string to be run on the remote host. e.g. echo 'hello world!'
        :return:
        """
        self.ip_file = ip_file
        self.start_date = start_date
        self.end_date = end_date
        self.period = period
        self.interval = interval
        self.command = command

        with open(ip_file) as f:
            self.target_list = f.read().splitlines()

        self.output_file_name = "schedule.txt"

    def create_schedule(self, seed=None):
        days = list(per_delta(self.start_date, self.end_date, timedelta(days=1)))
        with open(self.output_file_name, 'w') as outFile:
            for day in days:
                self.write_day(self.schedule_day(day, seed), outFile)

    def schedule_day(self, my_date: date, seed=None):
        day = {}
        d = datetime.combine(my_date, time(0, 0))
        if seed is not None:
            random.seed(seed)

        for period in per_delta(d, d+timedelta(days=1), self.period):
            # sample get me a random sample of the minutes in an hour ... up to 60
            # instead of putting that in the dictionary, we should put an actual date time
            # we should also pass in the day's date, and remove logic to make times consistent for the whole day
            # datetime removes a great deal of that calculation

            schedule = random.sample(list(per_delta(period, (period + self.period), self.interval)), len(self.target_list))

            day.update(dict(zip(schedule, self.target_list)))

        return day


    def write_day(self,day: dict, outFile: fileinput):
        """Write the schedule for the day to a file"""

        items = [(k, v) for k, v in day.items()]
        items.sort()
        for line in items:
            string = "{0}\t{1:15s}\t{2:s}\n".format(line[0], line[1], self.command)
            outFile.write(string)


# taken from http://stackoverflow.com/questions/10688006/generate-a-list-of-datetimes-between-an-interval-in-python
def per_delta(start, end, delta):
    curr = start
    while curr < end:
        yield curr
        curr += delta

if __name__ == "__main__":
    scheduler = ScheduleMaker("ip_list.txt", date(2016, 3, 15), date(2016, 3, 17), timedelta(hours=1), timedelta(minutes=1), "echo 'Hello World!'")
    scheduler.create_schedule(0)
