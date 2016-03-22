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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds the results from an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `BaseResults`
--

LOCK TABLES `BaseResults` WRITE;
/*!40000 ALTER TABLE `BaseResults` DISABLE KEYS */;
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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds the common test parametes of an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `CommonData`
--

LOCK TABLES `CommonData` WRITE;
/*!40000 ALTER TABLE `CommonData` DISABLE KEYS */;
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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds the results from an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `DiscriminationResults`
--

LOCK TABLES `DiscriminationResults` WRITE;
/*!40000 ALTER TABLE `DiscriminationResults` DISABLE KEYS */;
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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds Experiment metadata and experimental results';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Experiments`
--

LOCK TABLES `Experiments` WRITE;
/*!40000 ALTER TABLE `Experiments` DISABLE KEYS */;
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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds meatadata about an experiment';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Metadata`
--

LOCK TABLES `Metadata` WRITE;
/*!40000 ALTER TABLE `Metadata` DISABLE KEYS */;
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
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Holds pcap data for each measuement';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pcap_data`
--

LOCK TABLES `pcap_data` WRITE;
/*!40000 ALTER TABLE `pcap_data` DISABLE KEYS */;
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

-- Dump completed on 2016-03-22  3:39:19
