-- MySQL dump 10.13  Distrib 5.5.47, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: testdb
-- ------------------------------------------------------
-- Server version	5.5.47-0ubuntu0.14.04.1

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
-- Table structure for table `BaseResults`
--

DROP TABLE IF EXISTS `BaseResults`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `BaseResults` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_Experiments` int(11) DEFAULT NULL,
  `base_time` int(11) DEFAULT NULL COMMENT 'Non-Discrimination Time',
  `base_losses_str` varchar(512) DEFAULT NULL COMMENT 'string of missing packets',
  `base_src_port` int(6) DEFAULT NULL COMMENT 'base test source port',
  `base_dest_port` int(6) DEFAULT NULL COMMENT 'base test destination port',
  `base_tos` int(4) DEFAULT NULL COMMENT 'Base test TOS',
  `base_id` int(11) DEFAULT NULL,
  `base_ttl` int(11) DEFAULT NULL,
  `base_frag_off` int(11) DEFAULT NULL,
  `base_filename` varchar(128) DEFAULT NULL,
  `id_pcap_data` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `id_Experiments` (`id_Experiments`),
  CONSTRAINT `BaseResults_ibfk_1` FOREIGN KEY (`id_Experiments`) REFERENCES `Experiments` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=100 DEFAULT CHARSET=latin1 COMMENT='Holds the results from an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `BaseResults`
--

LOCK TABLES `BaseResults` WRITE;
/*!40000 ALTER TABLE `BaseResults` DISABLE KEYS */;
INSERT INTO `BaseResults` VALUES (1,1,2,'1-10',22,9999,69,0,33,0,'0',1),(2,63,10,'0',22222,15555,0,0,2560,0,'0',0),(3,64,10,'0',22222,15555,0,0,2560,0,'/dev/zero',0),(4,65,11,'0',22222,15555,0,0,2560,0,'/dev/zero',0),(5,66,10,'0',22222,15555,0,0,2560,0,'/dev/zero',0),(6,67,10,'0',22222,15555,0,0,2560,0,'/dev/zero',0),(7,81,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(8,82,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(9,83,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(10,84,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(11,85,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(12,86,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(13,87,10,'0',22222,15555,0,0,10,0,'/dev/zero',0),(14,89,9,'0',22222,15555,0,0,10,0,'/dev/zero',0),(15,90,9,'0',22222,15555,0,0,10,0,'/dev/zero',0),(16,91,9,'0',22222,15555,0,0,10,0,'/dev/zero',0),(17,95,10,'500',20000,22223,0,0,10,0,'/dev/zero',105),(18,98,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(19,99,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(20,100,10,'0',20000,22223,0,0,10,0,'/dev/zero',110),(21,101,10,'0',20000,22223,0,0,10,0,'/dev/zero',113),(22,102,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(23,103,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(24,104,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(25,105,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(26,106,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(27,107,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(28,108,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(29,109,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(30,112,9,'0',20000,22223,0,0,10,0,'/dev/zero',0),(31,113,10,'500',20000,22223,0,0,10,0,'/dev/zero',0),(32,114,10,'0',20000,22223,0,0,10,0,'/dev/zero',0),(33,115,10,'0',20000,22223,0,0,10,0,'/dev/zero',128),(34,116,10,'0',20000,22223,0,0,10,0,'/dev/zero',131),(35,117,9,'0',20000,22223,0,0,10,0,'/dev/zero',134),(36,118,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(37,120,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(38,121,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(39,122,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(40,123,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(41,124,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(42,125,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(43,126,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(44,127,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(45,128,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(46,129,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(47,130,0,'0',20000,22223,0,0,10,0,'/dev/zero',0),(48,131,14,'0',20000,22223,0,0,10,0,'/dev/zero',149),(49,132,15,'0',20000,22223,0,0,10,0,'/dev/zero',151),(50,133,12,'500',20000,22223,0,0,10,0,'/dev/zero',153),(51,134,12,'0',20000,22223,0,0,10,0,'/dev/zero',0),(52,135,9,'0',20000,22223,0,0,10,0,'/dev/zero',156),(53,136,11,'0',20000,22223,0,0,10,0,'/dev/zero',159),(54,137,10,'0',20000,22223,0,0,10,0,'/dev/zero',162),(55,138,9,'0',20000,22223,0,0,10,0,'/dev/zero',165),(56,139,9,'0',20000,22223,0,0,10,0,'/dev/zero',168),(57,140,10,'0',20000,22223,0,0,10,0,'/dev/zero',171),(58,141,18,'0',20000,22223,0,0,10,0,'/dev/zero',174),(59,142,9,'0',20000,22223,0,0,10,0,'/dev/zero',177),(60,143,13,'500',20000,22223,0,0,10,0,'/dev/zero',180),(61,144,20031,'0',20000,22223,0,0,10,0,'/dev/zero',183),(62,145,20016,'0',20000,22223,0,0,10,0,'/dev/zero',186),(63,146,20055,'0',20000,22222,0,0,10,0,'/dev/zero',189),(64,147,20030,'0',20000,22222,0,0,10,0,'/dev/zero',192),(65,148,20022,'0',20000,22222,0,0,10,0,'/dev/zero',195),(66,149,20039,'0',20000,22222,0,0,10,0,'/dev/zero',198),(67,150,20047,'0',20000,22222,0,0,10,0,'/dev/zero',201),(68,151,20010,'0',20000,22222,0,0,10,0,'/dev/zero',0),(69,152,20020,'500',20000,22222,0,0,10,0,'/dev/zero',0),(70,153,20019,'0',20000,22222,0,0,10,0,'/dev/zero',0),(71,154,20010,'0',20000,22222,0,0,10,0,'/dev/zero',0),(72,155,20011,'0',20000,22222,0,0,10,0,'/dev/zero',0),(73,156,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(74,157,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(75,158,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(76,159,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(77,160,50016,'0',20000,22222,0,0,10,0,'/dev/zero',0),(78,161,50014,'500',20000,22222,0,0,10,0,'/dev/zero',0),(79,162,50030,'500',20000,22222,0,0,10,0,'/dev/zero',0),(80,163,5002,'0',20000,22222,0,0,10,0,'/dev/zero',0),(81,164,5003,'500',20000,22222,0,0,10,0,'/dev/zero',0),(82,165,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(83,166,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(84,167,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(85,168,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(86,170,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(87,173,12054,'0',20000,22222,0,0,10,0,'/dev/zero',0),(88,174,5001,'0',20000,22222,0,0,10,0,'/dev/zero',227),(89,175,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(90,177,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(91,178,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(92,179,5002,'0',20000,22222,0,0,10,0,'/dev/zero',233),(93,180,5001,'0',20000,22222,0,0,10,0,'/dev/zero',236),(94,181,5004,'0',20000,22222,0,0,10,0,'/dev/zero',239),(95,182,5001,'0',20000,22222,0,0,10,0,'/dev/zero',242),(96,183,5002,'0',20000,22222,0,0,10,0,'/dev/zero',245),(97,184,5005,'0',20000,22222,0,0,10,0,'/dev/zero',248),(98,185,0,'0',20000,22222,0,0,10,0,'/dev/zero',0),(99,186,3004,'0',20000,22222,0,0,10,0,'/dev/zero',252);
/*!40000 ALTER TABLE `BaseResults` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `CommonData`
--

DROP TABLE IF EXISTS `CommonData`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `CommonData` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_Experiments` int(11) DEFAULT NULL,
  `protocol` varchar(16) DEFAULT NULL COMMENT 'Transport protocol',
  `num_packets` int(6) DEFAULT NULL COMMENT 'Number of packets sent',
  `num_tail` int(4) DEFAULT NULL COMMENT 'number of tail packets',
  `packet_size` int(6) DEFAULT NULL COMMENT 'size of payload sent',
  PRIMARY KEY (`id`),
  KEY `id_Experiments` (`id_Experiments`),
  CONSTRAINT `CommonData_ibfk_1` FOREIGN KEY (`id_Experiments`) REFERENCES `Experiments` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=143 DEFAULT CHARSET=latin1 COMMENT='Holds the common test parametes of an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `CommonData`
--

LOCK TABLES `CommonData` WRITE;
/*!40000 ALTER TABLE `CommonData` DISABLE KEYS */;
INSERT INTO `CommonData` VALUES (1,40,'udp',500,20,64),(2,42,'udp',500,20,64),(3,43,'udp',500,20,64),(4,44,'udp',500,20,64),(5,45,'udp',500,20,64),(6,46,'udp',500,20,64),(7,47,'udp',500,20,64),(8,48,'udp',500,20,64),(9,49,'udp',500,20,64),(10,50,'udp',500,20,64),(11,51,'udp',500,20,64),(12,52,'udp',500,20,64),(13,53,'udp',500,20,64),(14,54,'udp',500,20,64),(15,55,'udp',500,20,64),(16,56,'udp',500,20,64),(17,57,'udp',500,20,64),(18,58,'udp',500,20,64),(19,59,'udp',500,20,64),(20,60,'udp',500,20,64),(21,61,'udp',500,20,64),(22,62,'udp',500,20,64),(23,63,'udp',500,20,64),(24,64,'udp',500,20,64),(25,65,'udp',500,20,64),(26,66,'udp',500,20,64),(27,67,'udp',500,20,64),(28,68,'udp',500,20,64),(29,69,'udp',500,20,64),(30,70,'udp',500,20,64),(31,71,'udp',500,20,64),(32,72,'udp',500,20,64),(33,73,'udp',500,20,64),(34,74,'udp',500,20,64),(35,75,'udp',500,20,64),(36,76,'udp',500,20,64),(37,77,'udp',500,20,64),(38,78,'udp',500,20,64),(39,79,'udp',500,20,64),(40,80,'udp',500,20,64),(41,81,'udp',500,20,64),(42,82,'udp',500,20,64),(43,83,'udp',500,20,64),(44,84,'udp',500,20,64),(45,85,'udp',500,20,64),(46,86,'udp',500,20,64),(47,87,'udp',500,20,64),(48,88,'udp',500,20,64),(49,89,'udp',500,20,64),(50,90,'udp',500,20,64),(51,91,'udp',500,20,64),(52,92,'udp',500,20,64),(53,93,'udp',500,20,64),(54,94,'udp',500,20,64),(55,95,'udp',500,20,64),(56,98,'udp',500,20,64),(57,99,'udp',500,20,64),(58,100,'udp',500,20,64),(59,101,'udp',500,20,64),(60,102,'udp',500,20,64),(61,103,'udp',500,20,64),(62,104,'udp',500,20,64),(63,105,'udp',500,20,64),(64,106,'udp',500,20,64),(65,107,'udp',500,20,64),(66,108,'udp',500,20,64),(67,109,'udp',500,20,64),(68,110,'udp',500,20,64),(69,112,'udp',500,20,64),(70,113,'udp',500,20,64),(71,114,'udp',500,20,64),(72,115,'udp',500,20,64),(73,116,'udp',500,20,64),(74,117,'udp',500,20,64),(75,118,'udp',500,20,64),(76,120,'udp',500,20,64),(77,121,'udp',500,20,64),(78,122,'udp',500,20,64),(79,123,'udp',500,20,64),(80,124,'udp',500,20,64),(81,125,'udp',500,20,64),(82,126,'udp',500,20,64),(83,127,'udp',500,20,64),(84,128,'udp',500,20,64),(85,129,'udp',500,20,64),(86,130,'udp',500,20,64),(87,131,'udp',500,20,64),(88,132,'udp',500,20,64),(89,133,'udp',500,20,64),(90,134,'udp',500,20,64),(91,135,'udp',500,20,64),(92,136,'udp',500,20,64),(93,137,'udp',500,20,64),(94,138,'udp',500,20,64),(95,139,'udp',500,20,64),(96,140,'udp',500,20,64),(97,141,'udp',500,20,64),(98,142,'udp',500,20,64),(99,143,'udp',500,20,64),(100,144,'udp',500,20,64),(101,145,'udp',500,20,64),(102,146,'udp',500,20,64),(103,147,'udp',500,20,64),(104,148,'udp',500,20,64),(105,149,'udp',500,20,64),(106,150,'udp',500,20,64),(107,151,'udp',500,20,64),(108,152,'udp',500,20,64),(109,153,'udp',500,20,64),(110,154,'udp',500,20,64),(111,155,'udp',500,20,64),(112,156,'udp',500,20,64),(113,157,'udp',500,20,64),(114,158,'udp',500,20,64),(115,159,'udp',500,20,64),(116,160,'udp',500,20,64),(117,161,'udp',500,20,64),(118,162,'udp',500,20,64),(119,163,'udp',500,20,64),(120,164,'udp',500,20,64),(121,165,'udp',500,20,64),(122,166,'udp',500,20,64),(123,167,'udp',500,20,64),(124,168,'udp',500,20,64),(125,169,'udp',500,20,64),(126,170,'udp',500,20,64),(127,171,'udp',500,20,64),(128,172,'udp',500,20,64),(129,173,'udp',500,20,64),(130,174,'udp',500,20,64),(131,175,'udp',500,20,64),(132,176,'udp',500,20,64),(133,177,'udp',500,20,64),(134,178,'udp',500,20,64),(135,179,'udp',500,20,64),(136,180,'udp',500,20,64),(137,181,'udp',500,20,64),(138,182,'udp',500,20,64),(139,183,'udp',500,20,64),(140,184,'udp',500,20,64),(141,185,'udp',500,20,64),(142,186,'udp',500,20,64);
/*!40000 ALTER TABLE `CommonData` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `DiscriminationResults`
--

DROP TABLE IF EXISTS `DiscriminationResults`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `DiscriminationResults` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_Experiments` int(11) DEFAULT NULL,
  `discrimination_time` double DEFAULT NULL COMMENT 'Discrimination Time',
  `discrimination_losses_str` varchar(512) DEFAULT NULL COMMENT 'missing packets ',
  `disc_src_port` int(6) DEFAULT NULL COMMENT 'discrimination test source port',
  `disc_dest_port` int(6) DEFAULT NULL COMMENT 'Discrimination test source port',
  `disc_tos` int(11) DEFAULT NULL,
  `disc_id` int(11) DEFAULT NULL,
  `disc_frag_off` int(11) DEFAULT NULL,
  `disc_ttl` int(11) DEFAULT NULL,
  `disc_filename` varchar(128) DEFAULT NULL,
  `id_pcap_data` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `id_Experiments` (`id_Experiments`),
  CONSTRAINT `DiscriminationResults_ibfk_1` FOREIGN KEY (`id_Experiments`) REFERENCES `Experiments` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=latin1 COMMENT='Holds the results from an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `DiscriminationResults`
--

LOCK TABLES `DiscriminationResults` WRITE;
/*!40000 ALTER TABLE `DiscriminationResults` DISABLE KEYS */;
INSERT INTO `DiscriminationResults` VALUES (1,86,37.2488,'0',22223,15555,0,0,10,0,'/dev/urandom',0),(2,87,9.59824,'0',22223,15555,0,0,10,0,'/dev/urandom',0),(3,89,17.5131,'0',22223,15555,0,0,10,0,'/dev/urandom',0),(4,90,12.7708,'0',22223,15555,0,0,10,0,'/dev/urandom',0),(5,91,28.5761,'0',22223,15555,0,0,10,0,'/dev/urandom',0),(6,95,29.588,'500',20000,22223,0,0,10,0,'/dev/urandom',106),(7,100,9.47415,'500',20000,22223,0,0,10,0,'/dev/urandom',111),(8,101,9.63122,'500',20000,22223,0,0,10,0,'/dev/urandom',114),(9,112,40.0406,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(10,113,8.79094,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(11,114,9.38495,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(12,115,9.62814,'500',20000,22223,0,0,10,0,'/dev/urandom',129),(13,116,41.4213,'500',20000,22223,0,0,10,0,'/dev/urandom',132),(14,117,14.0087,'500',20000,22223,0,0,10,0,'/dev/urandom',135),(15,135,31.8651,'500',20000,22223,0,0,10,0,'/dev/urandom',157),(16,136,12.582,'500',20000,22223,0,0,10,0,'/dev/urandom',160),(17,137,31.2412,'500',20000,22223,0,0,10,0,'/dev/urandom',163),(18,138,11.9858,'500',20000,22223,0,0,10,0,'/dev/urandom',166),(19,139,12.7234,'500',20000,22223,0,0,10,0,'/dev/urandom',169),(20,140,9.80248,'500',20000,22223,0,0,10,0,'/dev/urandom',172),(21,141,15.6627,'500',20000,22223,0,0,10,0,'/dev/urandom',175),(22,142,43.2223,'500',20000,22223,0,0,10,0,'/dev/urandom',178),(23,143,12.8725,'500',20000,22223,0,0,10,0,'/dev/urandom',181),(24,144,20019.7,'500',20000,22223,0,0,10,0,'/dev/urandom',184),(25,145,20043,'500',20000,22223,0,0,10,0,'/dev/urandom',187),(26,146,20016.5,'500',20000,22223,0,0,10,0,'/dev/urandom',190),(27,147,20036.5,'500',20000,22223,0,0,10,0,'/dev/urandom',193),(28,148,20012.3,'500',20000,22223,0,0,10,0,'/dev/urandom',196),(29,149,20049,'500',20000,22223,0,0,10,0,'/dev/urandom',199),(30,150,20045.3,'500',20000,22223,0,0,10,0,'/dev/urandom',202),(31,151,20010,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(32,152,20013.9,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(33,153,20018.9,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(34,154,20009,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(35,155,20038.7,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(36,160,50034.5,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(37,161,50044.8,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(38,162,50014,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(39,163,5003.89,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(40,164,5001.45,'500',20000,22223,0,0,10,0,'/dev/urandom',0),(41,173,0,'0',20000,22223,0,0,10,0,'/dev/urandom',0),(42,174,0,'0',20000,22223,0,0,10,0,'/dev/urandom',0),(43,179,5002.91,'0',20000,22223,0,0,10,0,'/dev/urandom',234),(44,180,5004.02,'0',20000,22223,0,0,10,0,'/dev/urandom',237),(45,181,5001.12,'0',20000,22223,0,0,10,0,'/dev/urandom',240),(46,182,5004.02,'0',20000,22223,0,0,10,0,'/dev/urandom',243),(47,183,5004.93,'0',20000,22223,0,0,10,0,'/dev/urandom',246),(48,184,5001.33,'0',20000,22223,0,0,10,0,'/dev/urandom',249),(49,186,2003.34,'0',20000,22223,0,0,10,0,'/dev/urandom',253);
/*!40000 ALTER TABLE `DiscriminationResults` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Experiments`
--

DROP TABLE IF EXISTS `Experiments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Experiments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=187 DEFAULT CHARSET=latin1 COMMENT='Holds Experiment metadata and experimental results';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Experiments`
--

LOCK TABLES `Experiments` WRITE;
/*!40000 ALTER TABLE `Experiments` DISABLE KEYS */;
INSERT INTO `Experiments` VALUES (1),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13),(14),(15),(16),(17),(18),(19),(20),(21),(22),(23),(24),(25),(26),(27),(28),(29),(30),(31),(32),(33),(34),(35),(36),(37),(38),(39),(40),(41),(42),(43),(44),(45),(46),(47),(48),(49),(50),(51),(52),(53),(54),(55),(56),(57),(58),(59),(60),(61),(62),(63),(64),(65),(66),(67),(68),(69),(70),(71),(72),(73),(74),(75),(76),(77),(78),(79),(80),(81),(82),(83),(84),(85),(86),(87),(88),(89),(90),(91),(92),(93),(94),(95),(96),(97),(98),(99),(100),(101),(102),(103),(104),(105),(106),(107),(108),(109),(110),(111),(112),(113),(114),(115),(116),(117),(118),(119),(120),(121),(122),(123),(124),(125),(126),(127),(128),(129),(130),(131),(132),(133),(134),(135),(136),(137),(138),(139),(140),(141),(142),(143),(144),(145),(146),(147),(148),(149),(150),(151),(152),(153),(154),(155),(156),(157),(158),(159),(160),(161),(162),(163),(164),(165),(166),(167),(168),(169),(170),(171),(172),(173),(174),(175),(176),(177),(178),(179),(180),(181),(182),(183),(184),(185),(186);
/*!40000 ALTER TABLE `Experiments` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Metadata`
--

DROP TABLE IF EXISTS `Metadata`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Metadata` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_Experiments` int(11) DEFAULT NULL,
  `Project` varchar(64) DEFAULT NULL,
  `test_name` varchar(64) DEFAULT NULL,
  `test_date` datetime DEFAULT NULL,
  `command` varchar(512) DEFAULT NULL,
  `host_ip` varchar(128) DEFAULT NULL,
  `dest_ip` varchar(128) DEFAULT NULL,
  `success` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `id_Experiments` (`id_Experiments`),
  CONSTRAINT `Metadata_ibfk_1` FOREIGN KEY (`id_Experiments`) REFERENCES `Experiments` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=151 DEFAULT CHARSET=latin1 COMMENT='Holds meatadata about an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Metadata`
--

LOCK TABLES `Metadata` WRITE;
/*!40000 ALTER TABLE `Metadata` DISABLE KEYS */;
INSERT INTO `Metadata` VALUES (1,32,'Thesis','Policing--Co-Op','2016-03-20 17:58:49','client --test_id_in=32','127.0.0.1','127.0.0.1',0),(2,33,'Thesis','Policing--Co-Op','2016-03-20 17:58:55','client --test_id_in=33','127.0.0.1','127.0.0.1',0),(3,34,'Thesis','Policing--Co-Op','2016-03-20 18:00:17','client --test_id_in=34','192.168.1.100','127.0.0.1',0),(4,35,'Thesis','Policing--Co-Op','2016-03-20 18:03:15','client --test_id_in=35','192.168.1.100','127.0.0.1',0),(5,36,'Thesis','Policing--Co-Op','2016-03-20 18:04:19','client --test_id_in=36','192.168.1.100','127.0.0.1',1),(6,37,'Thesis','Policing--Co-Op','2016-03-20 19:15:22','client --test_id_in=37','192.168.1.100','127.0.0.1',1),(7,38,'Thesis','Policing--Co-Op','2016-03-20 19:44:00','client --test_id_in=38','192.168.1.100','127.0.0.1',1),(8,39,'Thesis','Policing--Co-Op','2016-03-20 19:44:31','client --test_id_in=39','192.168.1.100','127.0.0.1',1),(9,40,'Thesis','Policing--Co-Op','2016-03-20 19:45:24','client --test_id_in=40','192.168.1.100','127.0.0.1',1),(10,42,'Thesis','Policing--Co-Op','2016-03-20 21:01:50','client --test_id_in=42','192.168.1.100','127.0.0.1',1),(11,43,'Thesis','Policing--Co-Op','2016-03-20 21:02:21','client --test_id_in=43','192.168.1.100','127.0.0.1',1),(12,44,'Thesis','Policing--Co-Op','2016-03-20 21:03:13','client --test_id_in=44','192.168.1.100','127.0.0.1',1),(13,45,'Thesis','Policing--Co-Op','2016-03-20 21:03:29','client --test_id_in=45','192.168.1.100','127.0.0.1',1),(14,46,'Thesis','Policing--Co-Op','2016-03-20 21:03:53','client --test_id_in=46','192.168.1.100','127.0.0.1',1),(15,47,'Thesis','Policing--Co-Op','2016-03-20 21:06:45','client --test_id_in=47','192.168.1.100','127.0.0.1',1),(16,48,'Thesis','Policing--Co-Op','2016-03-20 21:08:16','client --test_id_in=48','192.168.1.100','127.0.0.1',1),(17,49,'Thesis','Policing--Co-Op','2016-03-20 21:14:49','client --test_id_in=49','192.168.1.100','127.0.0.1',1),(18,50,'Thesis','Policing--Co-Op','2016-03-20 21:17:34','client --test_id_in=50','192.168.1.100','127.0.0.1',1),(19,51,'Thesis','Policing--Co-Op','2016-03-20 21:21:04','client --test_id_in=51','192.168.1.100','127.0.0.1',1),(20,52,'Thesis','Policing--Co-Op','2016-03-20 21:34:00','client --test_id_in=52','192.168.1.100','127.0.0.1',1),(21,53,'Thesis','Policing--Co-Op','2016-03-20 21:34:33','client --test_id_in=53','192.168.1.100','127.0.0.1',1),(22,54,'Thesis','Policing--Co-Op','2016-03-20 21:35:01','client --test_id_in=54','192.168.1.100','127.0.0.1',1),(23,55,'Thesis','Policing--Co-Op','2016-03-20 21:35:39','client --test_id_in=55','192.168.1.100','127.0.0.1',1),(24,56,'Thesis','Policing--Co-Op','2016-03-20 21:35:55','client --test_id_in=56','192.168.1.100','127.0.0.1',1),(25,57,'Thesis','Policing--Co-Op','2016-03-20 21:51:19','client --test_id_in=57','192.168.1.100','127.0.0.1',1),(26,58,'Thesis','Policing--Co-Op','2016-03-20 21:52:22','client --test_id_in=58','192.168.1.100','127.0.0.1',1),(27,59,'Thesis','Policing--Co-Op','2016-03-20 21:53:32','client --test_id_in=59','192.168.1.100','127.0.0.1',1),(28,60,'Thesis','Policing--Co-Op','2016-03-20 21:53:48','client --test_id_in=60','192.168.1.100','127.0.0.1',1),(29,61,'Thesis','Policing--Co-Op','2016-03-20 21:54:42','client --test_id_in=61','192.168.1.100','127.0.0.1',1),(30,62,'Thesis','Policing--Co-Op','2016-03-20 21:56:24','client --test_id_in=62','192.168.1.100','127.0.0.1',1),(31,63,'Thesis','Policing--Co-Op','2016-03-20 22:00:39','client --test_id_in=63','192.168.1.100','127.0.0.1',1),(32,64,'Thesis','Policing--Co-Op','2016-03-20 22:03:38','client --test_id_in=64','192.168.1.100','127.0.0.1',1),(33,65,'Thesis','Policing--Co-Op','2016-03-20 22:04:40','client --test_id_in=65','192.168.1.100','127.0.0.1',1),(34,66,'Thesis','Policing--Co-Op','2016-03-20 22:05:04','client --test_id_in=66','192.168.1.100','127.0.0.1',1),(35,67,'Thesis','Policing--Co-Op','2016-03-20 22:05:08','client --test_id_in=67','192.168.1.100','127.0.0.1',1),(36,68,'Thesis','Policing--Co-Op','2016-03-20 22:07:23','client --test_id_in=68','192.168.1.100','127.0.0.1',1),(37,69,'Thesis','Policing--Co-Op','2016-03-20 22:09:49','client --test_id_in=69','192.168.1.100','127.0.0.1',1),(38,70,'Thesis','Policing--Co-Op','2016-03-20 22:10:27','client --test_id_in=70','192.168.1.100','127.0.0.1',1),(39,71,'Thesis','Policing--Co-Op','2016-03-20 22:11:42','client --test_id_in=71','192.168.1.100','127.0.0.1',1),(40,72,'Thesis','Policing--Co-Op','2016-03-20 22:14:35','client --test_id_in=72','192.168.1.100','127.0.0.1',1),(41,73,'Thesis','Policing--Co-Op','2016-03-20 22:18:27','client --test_id_in=73','192.168.1.100','127.0.0.1',1),(42,74,'Thesis','Policing--Co-Op','2016-03-20 22:19:36','client --test_id_in=74','192.168.1.100','127.0.0.1',1),(43,75,'Thesis','Policing--Co-Op','2016-03-20 22:20:41','client --test_id_in=75','192.168.1.100','127.0.0.1',1),(44,76,'Thesis','Policing--Co-Op','2016-03-20 22:20:53','client --test_id_in=76','192.168.1.100','127.0.0.1',1),(45,77,'Thesis','Policing--Co-Op','2016-03-20 22:21:23','client --test_id_in=77','192.168.1.100','127.0.0.1',1),(46,78,'Thesis','Policing--Co-Op','2016-03-20 22:22:12','client --test_id_in=78','192.168.1.100','127.0.0.1',1),(47,79,'Thesis','Policing--Co-Op','2016-03-20 22:23:13','client --test_id_in=79','192.168.1.100','127.0.0.1',1),(48,80,'Thesis','Policing--Co-Op','2016-03-20 22:24:25','client --test_id_in=80','192.168.1.100','127.0.0.1',1),(49,81,'Thesis','Policing--Co-Op','2016-03-20 22:25:20','client --test_id_in=81','192.168.1.100','127.0.0.1',1),(50,82,'Thesis','Policing--Co-Op','2016-03-20 22:27:21','client --test_id_in=82','192.168.1.100','127.0.0.1',1),(51,83,'Thesis','Policing--Co-Op','2016-03-20 22:28:25','client --test_id_in=83','192.168.1.100','127.0.0.1',1),(52,84,'Thesis','Policing--Co-Op','2016-03-20 22:32:04','client --test_id_in=84','192.168.1.100','127.0.0.1',1),(53,85,'Thesis','Policing--Co-Op','2016-03-20 22:32:39','client --test_id_in=85','192.168.1.100','127.0.0.1',1),(54,86,'Thesis','Policing--Co-Op','2016-03-20 22:33:11','client --test_id_in=86','192.168.1.100','127.0.0.1',1),(55,87,'Thesis','Policing--Co-Op','2016-03-20 22:35:49','client --test_id_in=87','192.168.1.100','127.0.0.1',1),(56,88,'Thesis','Policing--Co-Op','2016-03-20 22:36:04','client --test_id_in=88','192.168.1.100','127.0.0.1',1),(57,89,'Thesis','Policing--Co-Op','2016-03-20 22:37:23','client --test_id_in=89','192.168.1.100','127.0.0.1',1),(58,90,'Thesis','Policing--Co-Op','2016-03-20 22:42:01','client --test_id_in=90','192.168.1.100','127.0.0.1',1),(59,91,'Thesis','Policing--Co-Op','2016-03-20 23:47:10','client --test_id_in=91','192.168.1.100','127.0.0.1',1),(60,92,'Thesis','Policing--Co-Op','2016-03-21 01:33:46','client --test_id_in=92','192.168.1.100','127.0.0.1',0),(61,93,'Thesis','Policing--Co-Op','2016-03-21 12:29:00','client --test_id_in=93','192.168.1.100','127.0.0.1',0),(62,94,'Thesis','Policing--Co-Op','2016-03-21 13:21:27','client --test_id_in=94','192.168.1.100','127.0.0.1',0),(63,95,'Thesis','Policing--Co-Op','2016-03-21 13:25:12','client --test_id_in=95','192.168.1.100','127.0.0.1',1),(64,98,'Thesis','Policing--Co-Op','2016-03-21 13:38:17','client --test_id_in=98','192.168.1.100','127.0.0.1',0),(65,99,'Thesis','Policing--Co-Op','2016-03-21 13:43:13','client --test_id_in=99','192.168.1.100','127.0.0.1',0),(66,100,'Thesis','Policing--Co-Op','2016-03-21 13:45:48','client --test_id_in=100','192.168.1.100','127.0.0.1',1),(67,101,'Thesis','Policing--Co-Op','2016-03-21 13:47:14','client --test_id_in=101','192.168.1.100','127.0.0.1',1),(68,102,'Thesis','Policing--Co-Op','2016-03-21 13:49:19','client --test_id_in=102','192.168.1.100','127.0.0.1',0),(69,103,'Thesis','Policing--Co-Op','2016-03-21 13:52:20','client --test_id_in=103','192.168.1.100','127.0.0.1',0),(70,104,'Thesis','Policing--Co-Op','2016-03-21 13:52:20','client --test_id_in=104','192.168.1.100','127.0.0.1',0),(71,105,'Thesis','Policing--Co-Op','2016-03-21 13:57:29','client --test_id_in=105','192.168.1.100','127.0.0.1',0),(72,106,'Thesis','Policing--Co-Op','2016-03-21 14:02:21','client --test_id_in=106','192.168.1.100','127.0.0.1',0),(73,107,'Thesis','Policing--Co-Op','2016-03-21 14:03:52','client --test_id_in=107','192.168.1.100','127.0.0.1',0),(74,108,'Thesis','Policing--Co-Op','2016-03-21 14:06:09','client --test_id_in=108','192.168.1.100','127.0.0.1',0),(75,109,'Thesis','Policing--Co-Op','2016-03-21 14:08:54','client --test_id_in=109','192.168.1.100','127.0.0.1',0),(76,110,'Thesis','Policing--Co-Op','2016-03-21 14:09:50','client --test_id_in=110','192.168.1.100','127.0.0.1',0),(77,112,'Thesis','Policing--Co-Op','2016-03-21 14:15:37','client --test_id_in=112','192.168.1.100','127.0.0.1',1),(78,113,'Thesis','Policing--Co-Op','2016-03-21 14:17:33','client --test_id_in=113','192.168.1.100','127.0.0.1',1),(79,114,'Thesis','Policing--Co-Op','2016-03-21 14:18:51','client --test_id_in=114','192.168.1.100','127.0.0.1',1),(80,115,'Thesis','Policing--Co-Op','2016-03-21 14:20:33','client --test_id_in=115','192.168.1.100','127.0.0.1',1),(81,116,'Thesis','Policing--Co-Op','2016-03-21 14:21:44','client --test_id_in=116','192.168.1.100','127.0.0.1',1),(82,117,'Thesis','Policing--Co-Op','2016-03-21 14:24:18','client --test_id_in=117','192.168.1.100','127.0.0.1',1),(83,118,'Thesis','Policing--Co-Op','2016-03-21 14:35:45','client --test_id_in=118','192.168.1.100','127.0.0.1',0),(84,120,'Thesis','Policing--Co-Op','2016-03-21 14:52:51','client --test_id_in=120','192.168.1.100','127.0.0.1',1),(85,121,'Thesis','Policing--Co-Op','2016-03-21 14:53:56','client --test_id_in=121','192.168.1.100','127.0.0.1',1),(86,122,'Thesis','Policing--Co-Op','2016-03-21 14:56:52','client --test_id_in=122','192.168.1.100','127.0.0.1',1),(87,123,'Thesis','Policing--Co-Op','2016-03-21 14:58:52','client --test_id_in=123','192.168.1.100','127.0.0.1',1),(88,124,'Thesis','Policing--Co-Op','2016-03-21 15:01:52','client --test_id_in=124','192.168.1.100','127.0.0.1',1),(89,125,'Thesis','Policing--Co-Op','2016-03-21 15:04:05','client --test_id_in=125','192.168.1.100','127.0.0.1',1),(90,126,'Thesis','Policing--Co-Op','2016-03-21 15:08:22','client --test_id_in=126','192.168.1.100','127.0.0.1',1),(91,127,'Thesis','Policing--Co-Op','2016-03-21 15:09:27','client --test_id_in=127','192.168.1.100','127.0.0.1',1),(92,128,'Thesis','Policing--Co-Op','2016-03-21 15:10:46','client --test_id_in=128','192.168.1.100','127.0.0.1',1),(93,129,'Thesis','Policing--Co-Op','2016-03-21 15:17:59','client --test_id_in=129','192.168.1.100','127.0.0.1',1),(94,130,'Thesis','Policing--Co-Op','2016-03-21 15:20:30','client --test_id_in=130','192.168.1.100','127.0.0.1',1),(95,131,'Thesis','Policing--Co-Op','2016-03-21 15:36:37','client --test_id_in=131','192.168.1.100','127.0.0.1',1),(96,132,'Thesis','Policing--Co-Op','2016-03-21 15:41:05','client --test_id_in=132','192.168.1.100','127.0.0.1',1),(97,133,'Thesis','Policing--Co-Op','2016-03-21 15:41:49','client --test_id_in=133','192.168.1.100','127.0.0.1',1),(98,134,'Thesis','Policing--Co-Op','2016-03-21 15:43:41','client --test_id_in=134','192.168.1.100','127.0.0.1',1),(99,135,'Thesis','Policing--Co-Op','2016-03-21 15:47:11','client --test_id_in=135','192.168.1.100','127.0.0.1',1),(100,136,'Thesis','Policing--Co-Op','2016-03-21 15:48:27','client --test_id_in=136','192.168.1.100','127.0.0.1',1),(101,137,'Thesis','Policing--Co-Op','2016-03-21 15:49:43','client --test_id_in=137','192.168.1.100','127.0.0.1',1),(102,138,'Thesis','Policing--Co-Op','2016-03-21 15:51:08','client --test_id_in=138','192.168.1.100','127.0.0.1',1),(103,139,'Thesis','Policing--Co-Op','2016-03-21 15:53:17','client --test_id_in=139','192.168.1.100','127.0.0.1',1),(104,140,'Thesis','Policing--Co-Op','2016-03-21 15:55:02','client --test_id_in=140','192.168.1.100','127.0.0.1',1),(105,141,'Thesis','Policing--Co-Op','2016-03-21 15:55:48','client --test_id_in=141','192.168.1.100','127.0.0.1',1),(106,142,'Thesis','Policing--Co-Op','2016-03-21 15:56:56','client --test_id_in=142','192.168.1.100','127.0.0.1',1),(107,143,'Thesis','Policing--Co-Op','2016-03-21 15:57:12','client --test_id_in=143','192.168.1.100','127.0.0.1',1),(108,144,'Thesis','Policing--Co-Op','2016-03-21 16:16:13','client --test_id_in=144','192.168.1.100','127.0.0.1',1),(109,145,'Thesis','Policing--Co-Op','2016-03-21 16:18:37','client --test_id_in=145','192.168.1.100','127.0.0.1',1),(110,146,'Thesis','Policing--Co-Op','2016-03-21 16:20:15','client --test_id_in=146','192.168.1.100','127.0.0.1',1),(111,147,'Thesis','Policing--Co-Op','2016-03-21 16:22:39','client --test_id_in=147','192.168.1.100','127.0.0.1',1),(112,148,'Thesis','Policing--Co-Op','2016-03-21 16:23:57','client --test_id_in=148','192.168.1.100','127.0.0.1',1),(113,149,'Thesis','Policing--Co-Op','2016-03-21 16:25:29','client --test_id_in=149','192.168.1.100','127.0.0.1',1),(114,150,'Thesis','Policing--Co-Op','2016-03-21 21:03:40','client --test_id_in=150','192.168.1.100','127.0.0.1',1),(115,151,'Thesis','Policing--Co-Op','2016-03-21 21:25:44','client --test_id_in=151','192.168.1.100','127.0.0.1',1),(116,152,'Thesis','Policing--Co-Op','2016-03-21 21:27:18','client --test_id_in=152','192.168.1.100','127.0.0.1',1),(117,153,'Thesis','Policing--Co-Op','2016-03-21 21:38:44','client --test_id_in=153','192.168.1.100','127.0.0.1',1),(118,154,'Thesis','Policing--Co-Op','2016-03-21 21:41:39','client --test_id_in=154','192.168.1.100','127.0.0.1',1),(119,155,'Thesis','Policing--Co-Op','2016-03-21 21:42:56','client --test_id_in=155','192.168.1.100','127.0.0.1',1),(120,156,'Thesis','Policing--Co-Op','2016-03-21 21:45:46','client --test_id_in=156','192.168.1.100','127.0.0.1',0),(121,157,'Thesis','Policing--Co-Op','2016-03-21 21:55:03','client --test_id_in=157','192.168.1.100','127.0.0.1',0),(122,158,'Thesis','Policing--Co-Op','2016-03-21 22:01:04','client --test_id_in=158','192.168.1.100','127.0.0.1',0),(123,159,'Thesis','Policing--Co-Op','2016-03-21 22:02:34','client --test_id_in=159','192.168.1.100','127.0.0.1',0),(124,160,'Thesis','Policing--Co-Op','2016-03-21 22:16:15','client --test_id_in=160','192.168.1.100','127.0.0.1',1),(125,161,'Thesis','Policing--Co-Op','2016-03-21 22:30:21','client --test_id_in=161','192.168.1.100','127.0.0.1',1),(126,162,'Thesis','Policing--Co-Op','2016-03-21 22:31:05','client --test_id_in=162','192.168.1.100','127.0.0.1',1),(127,163,'Thesis','Policing--Co-Op','2016-03-21 22:33:11','client --test_id_in=163','192.168.1.100','127.0.0.1',1),(128,164,'Thesis','Policing--Co-Op','2016-03-21 22:33:46','client --test_id_in=164','192.168.1.100','127.0.0.1',1),(129,165,'Thesis','Policing--Co-Op','2016-03-21 22:40:27','client --test_id_in=165','192.168.1.100','127.0.0.1',0),(130,166,'Thesis','Policing--Co-Op','2016-03-21 22:42:41','client --test_id_in=166','192.168.1.100','127.0.0.1',0),(131,167,'Thesis','Policing--Co-Op','2016-03-21 22:44:30','client --test_id_in=167','192.168.1.100','127.0.0.1',0),(132,168,'Thesis','Policing--Co-Op','2016-03-21 22:47:25','client --test_id_in=168','192.168.1.100','127.0.0.1',0),(133,169,'Thesis','Policing--Co-Op','2016-03-21 22:47:37','client --test_id_in=169','192.168.1.100','127.0.0.1',0),(134,170,'Thesis','Policing--Co-Op','2016-03-21 22:51:04','client --test_id_in=170','192.168.1.100','127.0.0.1',0),(135,171,'Thesis','Policing--Co-Op','2016-03-21 22:51:35','client --test_id_in=171','192.168.1.100','127.0.0.1',0),(136,172,'Thesis','Policing--Co-Op','2016-03-21 22:51:44','client --test_id_in=172','192.168.1.100','127.0.0.1',0),(137,173,'Thesis','Policing--Co-Op','2016-03-21 22:52:35','client --test_id_in=173','192.168.1.100','127.0.0.1',1),(138,174,'Thesis','Policing--Co-Op','2016-03-21 22:54:25','client --test_id_in=174','192.168.1.100','127.0.0.1',1),(139,175,'Thesis','Policing--Co-Op','2016-03-21 22:58:01','client --test_id_in=175','192.168.1.100','127.0.0.1',0),(140,176,'Thesis','Policing--Co-Op','2016-03-21 22:58:11','client --test_id_in=176','192.168.1.100','127.0.0.1',0),(141,177,'Thesis','Policing--Co-Op','2016-03-21 23:02:29','client --test_id_in=177','192.168.1.100','127.0.0.1',0),(142,178,'Thesis','Policing--Co-Op','2016-03-21 23:11:27','client --test_id_in=178','192.168.1.100','127.0.0.1',0),(143,179,'Thesis','Policing--Co-Op','2016-03-21 23:13:30','client --test_id_in=179','192.168.1.100','127.0.0.1',1),(144,180,'Thesis','Policing--Co-Op','2016-03-21 23:14:48','client --test_id_in=180','192.168.1.100','127.0.0.1',1),(145,181,'Thesis','Policing--Co-Op','2016-03-21 23:16:31','client --test_id_in=181','192.168.1.100','127.0.0.1',1),(146,182,'Thesis','Policing--Co-Op','2016-03-21 23:18:36','client --test_id_in=182','192.168.1.100','127.0.0.1',1),(147,183,'Thesis','Policing--Co-Op','2016-03-21 23:20:08','client --test_id_in=183','192.168.1.100','127.0.0.1',1),(148,184,'Thesis','Policing--Co-Op','2016-03-21 23:21:35','client --test_id_in=184','192.168.1.100','127.0.0.1',1),(149,185,'Thesis','Policing--Co-Op','2016-03-21 23:29:24','client --test_id_in=185','192.168.1.100','127.0.0.1',0),(150,186,'Thesis','Policing--Co-Op','2016-03-21 23:31:04','client --test_id_in=186','192.168.1.100','127.0.0.1',1);
/*!40000 ALTER TABLE `Metadata` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pcap_data`
--

DROP TABLE IF EXISTS `pcap_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pcap_data` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_Experiments` int(11) DEFAULT NULL,
  `Measurement Type` varchar(64) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `id_Experiments` (`id_Experiments`),
  CONSTRAINT `pcap_data_ibfk_1` FOREIGN KEY (`id_Experiments`) REFERENCES `Experiments` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=255 DEFAULT CHARSET=latin1 COMMENT='Holds pcap data for each measuement';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pcap_data`
--

LOCK TABLES `pcap_data` WRITE;
/*!40000 ALTER TABLE `pcap_data` DISABLE KEYS */;
INSERT INTO `pcap_data` VALUES (1,1,'base'),(2,1,'discrimination'),(14,1,'poop'),(15,1,'MY poopmysql -u rootmysql -u root'),(16,1,'MY poopmysql -u rootmysql -u root'),(17,1,'MY poop'),(18,1,'new poop'),(19,1,'new poop'),(20,1,'new poop'),(21,1,'new poop'),(22,1,'new poop'),(23,1,'new poop'),(24,1,'new poop'),(25,3,'new poop'),(26,4,'new poop'),(27,5,'new poop'),(28,6,'new poop'),(29,7,'new poop'),(30,8,'new poop'),(31,35,'Base'),(32,36,'Base'),(33,37,'Base'),(34,40,'Base'),(35,42,'Base'),(36,43,'Base'),(37,44,'Base'),(38,45,'Base'),(39,46,'Base'),(40,47,'Base'),(41,48,'Base'),(42,49,'Base'),(43,50,'Base'),(44,51,'Base'),(45,52,'Base'),(46,53,'Base'),(47,54,'Base'),(48,55,'Base'),(49,56,'Base'),(50,57,'Base'),(51,58,'Base'),(52,59,'Base'),(53,60,'Base'),(54,61,'Base'),(55,62,'Base'),(56,63,'Base'),(57,64,'Base'),(58,65,'Base'),(59,66,'Base'),(60,67,'Base'),(61,68,'Base'),(62,69,'Base'),(63,70,'Base'),(64,71,'Base'),(65,72,'Base'),(66,73,'Base'),(67,74,'Base'),(68,75,'Base'),(69,76,'Base'),(70,77,'Base'),(71,78,'Base'),(72,79,'Base'),(73,80,'Base'),(74,81,'Base'),(75,82,'Base'),(76,83,'Base'),(77,84,'Base'),(78,85,'Base'),(79,86,'Base'),(80,87,'Base'),(81,88,'Base'),(82,89,'Base'),(83,90,'Base'),(84,91,'Base'),(85,92,'Base'),(86,1,'new type'),(87,1,'new type'),(88,8,'new type'),(89,8,'new type'),(90,93,'Base'),(91,1,'new type'),(92,1,'new type'),(93,1,'new type'),(94,1,'new type'),(95,1,'new type'),(96,1,'new type'),(97,1,'new type'),(98,1,'new type'),(99,1,'new type'),(102,7,'new type'),(103,7,'new type'),(104,94,'Base'),(105,95,'new type'),(106,95,'new type'),(107,95,'Base'),(108,98,'Base'),(109,99,'Base'),(110,100,'new type'),(111,100,'new type'),(112,100,'Base'),(113,101,'new type'),(114,101,'new type'),(115,101,'Base'),(116,102,'Base'),(117,103,'Base'),(118,104,'Base'),(119,105,'Base'),(120,106,'Base'),(121,107,'Base'),(122,108,'Base'),(123,109,'Base'),(124,110,'Base'),(125,112,'Base'),(126,113,'Base'),(127,114,'Base'),(128,115,'new type'),(129,115,'new type'),(130,115,'Base'),(131,116,'new type'),(132,116,'new type'),(133,116,'Base'),(134,117,'new type'),(135,117,'new type'),(136,117,'Base'),(137,118,'Base'),(138,120,'Base'),(139,121,'Base'),(140,122,'Base'),(141,123,'Base'),(142,124,'Base'),(143,125,'Base'),(144,126,'Base'),(145,127,'Base'),(146,128,'Base'),(147,129,'Base'),(148,130,'Base'),(149,131,'new type'),(150,131,'Base'),(151,132,'new type'),(152,132,'Base'),(153,133,'new type'),(154,133,'Base'),(155,134,'Base'),(156,135,'new type'),(157,135,'new type'),(158,135,'Base'),(159,136,'new type'),(160,136,'new type'),(161,136,'Base'),(162,137,'new type'),(163,137,'new type'),(164,137,'Base'),(165,138,'new type'),(166,138,'new type'),(167,138,'Base'),(168,139,'new type'),(169,139,'new type'),(170,139,'Base'),(171,140,'new type'),(172,140,'new type'),(173,140,'Base'),(174,141,'new type'),(175,141,'new type'),(176,141,'Base'),(177,142,'new type'),(178,142,'new type'),(179,142,'Base'),(180,143,'new type'),(181,143,'new type'),(182,143,'Base'),(183,144,'new type'),(184,144,'new type'),(185,144,'Base'),(186,145,'new type'),(187,145,'new type'),(188,145,'Base'),(189,146,'new type'),(190,146,'new type'),(191,146,'Base'),(192,147,'new type'),(193,147,'new type'),(194,147,'Base'),(195,148,'new type'),(196,148,'new type'),(197,148,'Base'),(198,149,'new type'),(199,149,'new type'),(200,149,'Base'),(201,150,'new type'),(202,150,'new type'),(203,150,'Base'),(204,151,'Base'),(205,152,'Base'),(206,153,'Base'),(207,154,'Base'),(208,155,'Base'),(209,156,'Base'),(210,157,'Base'),(211,158,'Base'),(212,159,'Base'),(213,160,'Base'),(214,161,'Base'),(215,162,'Base'),(216,163,'Base'),(217,164,'Base'),(218,165,'Base'),(219,166,'Base'),(220,167,'Base'),(221,168,'Base'),(222,169,'Base'),(223,170,'Base'),(224,171,'Base'),(225,172,'Base'),(226,173,'Base'),(227,174,'new type'),(228,174,'Base'),(229,175,'Base'),(230,176,'Base'),(231,177,'Base'),(232,178,'Base'),(233,179,'new type'),(234,179,'new type'),(235,179,'Base'),(236,180,'new type'),(237,180,'new type'),(238,180,'Base'),(239,181,'new type'),(240,181,'new type'),(241,181,'Base'),(242,182,'new type'),(243,182,'new type'),(244,182,'Base'),(245,183,'new type'),(246,183,'new type'),(247,183,'Base'),(248,184,'new type'),(249,184,'new type'),(250,184,'Base'),(251,185,'Base'),(252,186,'new type'),(253,186,'new type'),(254,186,'Base');
/*!40000 ALTER TABLE `pcap_data` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-03-22  3:26:17
