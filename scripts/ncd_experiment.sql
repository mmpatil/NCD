-- MySQL dump 10.15  Distrib 10.0.21-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version	10.0.21-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `data`
--

DROP TABLE IF EXISTS `data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;

DROP TABLE IF EXISTS `experiment`;

CREATE TABLE `experiment` (
    `id` int(6)

);




CREATE TABLE `meta_data` (
  `id` int(6) unsigned NOT NULL AUTO_INCREMENT,
  `test_date` datetime DEFAULT NULL,
  `command_1` varchar(256) DEFAULT NULL,
  `host_ip` varchar(128) DEFAULT NULL,
  `dest_ip` varchar(128) DEFAULT NULL,
  `success` tinyint(1) DEFAULT '0',
  `test_name` varchar(128) DEFAULT NULL,
  `project` varchar(128) DEFAULT NULL,
  `high_time` double DEFAULT NULL,
  `low_time` double DEFAULT NULL,
  `high_losses_str` varchar(1000) DEFAULT NULL,
  `low_losses_str` varchar(1000) DEFAULT NULL,
 /* `high_losses` varchar(256) DEFAULT NULL,
  `low_losses` varchar(256) DEFAULT NULL,*/
  `num_tail` int(4) DEFAULT NULL,
  `dest_port` int(6) DEFAULT NULL,
  `src_port` int(6) DEFAULT NULL,
  `num_packets` int(6) DEFAULT NULL,
  `packet_size` int(6) DEFAULT NULL,
  `protocol` varchar(16) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `data`
--

LOCK TABLES `data` WRITE;
/*!40000 ALTER TABLE `data` DISABLE KEYS */;
INSERT INTO `data` VALUES (2,'2016-02-13 17:22:46','echo','127.0.0.1','8.8.8.8',1,'Prototype','NCD',25.001,2.275,'1-1000','7-500,502-1000',20,33333,22222,1000,1024,'UDP'),(3,'2016-02-15 19:32:18','echo','127.0.0.1','8.8.8.8',1,'Prototype','ncd',25.001,2.275,'1-1000','7-500,501-999',20,33333,2222,1000,1024,'UDP'),(4,'2016-02-15 20:28:05','poo','127.0.0.1','8.8.4.4',0,'fake_data','Tester',0.001,1.0005,'','',50,80,9876,69,512,'TCP');
/*!40000 ALTER TABLE `data` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-02-16 12:35:03
