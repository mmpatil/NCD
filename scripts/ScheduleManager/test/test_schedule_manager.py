import unittest
import filecmp
from datetime import *

from ScheduleManager import ScheduleMaker

class ScheduleMakerTestCase(unittest.TestCase):
    """Base test case for ScheduleMaker Test Cases"""

    def setUp(self):
        self.scheduler = ScheduleMaker("ip_list.txt", date(2016, 3,15), date(2016, 3, 17),timedelta(hours=1), timedelta(minutes=1),"echo 'Hello World!'")

    def tearDown(self):
        self.scheduler = None


class TestScheduleMakerInitCase(ScheduleMakerTestCase):
    """Test cases for the ScheduleMaker class"""

    def ruTest(self):
        self.assertEqual(self.scheduler.target_list, "ip_list.txt",  "Target IP List not set correctly")
        self.assertEqual(self.scheduler.start_date, date(2016, 3,15), "Start Date not Set correctly")
        self.assertEqual(self.scheduler.end_date, date(2016, 3, 16), "End Date not set correctly")
        self.assertEqual(self.scheduler.period, timedelta(hours=1),  "Period List not set correctly")
        self.assertEqual(self.scheduler.interval, timedelta(minutes=1),  "Interval List not set correctly")
        self.assertEqual(self.scheduler.command, "echo 'Hello",  "Command not set correctly")

class TestScheduleMarkerCreateScheduleCase(ScheduleMakerTestCase):
    def runTest(self):
        self.scheduler.create_schedule(0)
        self.assertTrue(filecmp.cmp("schedule.txt", "golden.txt"), "Files did not compare equal")