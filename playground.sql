-- MySQL dump 10.15  Distrib 10.0.21-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: playground
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
-- Table structure for table `metadata`
--

DROP TABLE IF EXISTS `metadata`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `metadata` (
  `test_id` int(5) NOT NULL AUTO_INCREMENT,
  `test_date` datetime DEFAULT NULL,
  `command` varchar(128) DEFAULT NULL,
  `host_ip` varchar(20) DEFAULT NULL,
  `target_ip` varchar(20) DEFAULT NULL,
  `success` tinyint(1) DEFAULT '0',
  `test_name` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`test_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `metadata`
--

LOCK TABLES `metadata` WRITE;
/*!40000 ALTER TABLE `metadata` DISABLE KEYS */;
INSERT INTO `metadata` VALUES (1,'2016-02-01 23:16:03','ncd 8.8.8.8','192.168.1.1','8.8.8.8',1,'First test');
/*!40000 ALTER TABLE `metadata` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ncd_data`
--

DROP TABLE IF EXISTS `ncd_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ncd_data` (
  `test_id` int(5) NOT NULL,
  `high_time` double DEFAULT '0',
  `high_losses` varchar(64) DEFAULT NULL,
  `low_time` double DEFAULT '0',
  `low_losses` varchar(64) DEFAULT NULL,
  KEY `test_id_ref` (`test_id`),
  CONSTRAINT `test_id_ref` FOREIGN KEY (`test_id`) REFERENCES `metadata` (`test_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ncd_data`
--

LOCK TABLES `ncd_data` WRITE;
/*!40000 ALTER TABLE `ncd_data` DISABLE KEYS */;
INSERT INTO `ncd_data` VALUES (1,25.112,NULL,2.75,NULL);
/*!40000 ALTER TABLE `ncd_data` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-02-05 13:00:37
