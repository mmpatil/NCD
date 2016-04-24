from __future__ import absolute_import
import unittest
import filecmp
from datetime import *

from ScheduleManager import ScheduleMaker

class ScheduleMakerTestCase(unittest.TestCase):
    u"""Base test case for ScheduleMaker Test Cases"""

    def setUp(self):
        self.scheduler = ScheduleMaker(u"ip_list.txt", date(2016, 3,15), date(2016, 3, 17),timedelta(hours=1), timedelta(minutes=1),u"echo 'Hello World!'")

    def tearDown(self):
        self.scheduler = None


class TestScheduleMakerInitCase(ScheduleMakerTestCase):
    u"""Test cases for the ScheduleMaker class"""

    def ruTest(self):
        self.assertEqual(self.scheduler.target_list, u"ip_list.txt",  u"Target IP List not set correctly")
        self.assertEqual(self.scheduler.start_date, date(2016, 3,15), u"Start Date not Set correctly")
        self.assertEqual(self.scheduler.end_date, date(2016, 3, 16), u"End Date not set correctly")
        self.assertEqual(self.scheduler.period, timedelta(hours=1),  u"Period List not set correctly")
        self.assertEqual(self.scheduler.interval, timedelta(minutes=1),  u"Interval List not set correctly")
        self.assertEqual(self.scheduler.command, u"echo 'Hello",  u"Command not set correctly")

class TestScheduleMarkerCreateScheduleCase(ScheduleMakerTestCase):
    def runTest(self):
        self.scheduler.create_schedule(0)
        self.assertTrue(filecmp.cmp(u"schedule.txt", u"golden.txt"), u"Files did not compare equal")