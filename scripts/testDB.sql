-- ---
-- Globals
-- ---

-- SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
-- SET FOREIGN_KEY_CHECKS=0;

-- ---
-- Table 'CommonData'
-- Holds the common test parametes of an experiment
-- ---
use testdb;
DROP TABLE IF EXISTS `CommonData`;

CREATE TABLE `CommonData` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  `id_Experiments` INTEGER NULL DEFAULT NULL,
  `protocol` VARCHAR(16) NULL DEFAULT NULL COMMENT 'Transport protocol',
  `num_packets` INT(6) NULL DEFAULT NULL COMMENT 'Number of packets sent',
  `num_tail` INT(4) NULL DEFAULT NULL COMMENT 'number of tail packets',
  `packet_size` INT(6) NULL DEFAULT NULL COMMENT 'size of payload sent',
  PRIMARY KEY (`id`)
) COMMENT 'Holds the common test parametes of an experiment';

-- ---
-- Table 'DiscriminationResults'
-- Holds the results from an experiment
-- ---

DROP TABLE IF EXISTS `DiscriminationResults`;

CREATE TABLE `DiscriminationResults` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  `id_Experiments` INTEGER NULL DEFAULT NULL,
  `discrimination_time` DOUBLE NULL DEFAULT NULL COMMENT 'Discrimination Time',
  `discrimination_losses_str` VARCHAR(512) NULL DEFAULT NULL COMMENT 'missing packets ',
  `disc_src_port` INT(6) NULL DEFAULT NULL COMMENT 'discrimination test source port',
  `disc_dest_port` INT(6) NULL DEFAULT NULL COMMENT 'Discrimination test source port',
  `disc_tos` INTEGER NULL DEFAULT NULL,
  `disc_id` INTEGER NULL DEFAULT NULL,
  `disc_frag_off` INTEGER NULL DEFAULT NULL,
  `disc_ttl` INTEGER NULL DEFAULT NULL,
  `disc_filename` INTEGER NULL DEFAULT NULL,
  `id_pcap_data` INTEGER NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) COMMENT 'Holds the results from an experiment';

-- ---
-- Table 'BaseResults'
-- Holds the results from an experiment
-- ---

DROP TABLE IF EXISTS `BaseResults`;

CREATE TABLE `BaseResults` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  `id_Experiments` INTEGER NULL DEFAULT NULL,
  `base_time` INTEGER NULL DEFAULT NULL COMMENT 'Non-Discrimination Time',
  `base_losses_str` VARCHAR(512) NULL DEFAULT NULL COMMENT 'string of missing packets',
  `base_src_port` INT(6) NULL DEFAULT NULL COMMENT 'base test source port',
  `base_dest_port` INT(6) NULL DEFAULT NULL COMMENT 'base test destination port',
  `base_tos` INT(4) NULL DEFAULT NULL COMMENT 'Base test TOS',
  `base_id` INTEGER NULL DEFAULT NULL,
  `base_ttl` INTEGER NULL DEFAULT NULL,
  `base_frag_off` INTEGER NULL DEFAULT NULL,
  `base_filename` INTEGER NULL DEFAULT NULL,
  `id_pcap_data` INTEGER NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) COMMENT 'Holds the results from an experiment';

-- ---
-- Table 'Experiments'
-- Holds Experiment metadata and experimental results
-- ---

DROP TABLE IF EXISTS `Experiments`;

CREATE TABLE `Experiments` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  PRIMARY KEY (`id`)
) COMMENT 'Holds Experiment metadata and experimental results';

-- ---
-- Table 'Metadata'
-- Holds meatadata about an experiment
-- ---

DROP TABLE IF EXISTS `Metadata`;

CREATE TABLE `Metadata` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  `id_Experiments` INTEGER NULL DEFAULT NULL,
  `Project` VARCHAR(64) NULL DEFAULT NULL,
  `test_name` VARCHAR(64) NULL DEFAULT NULL,
  `test_date` DATETIME NULL DEFAULT NULL,
  `command` VARCHAR(512) NULL DEFAULT NULL,
  `host_ip` VARCHAR(128) NULL DEFAULT NULL,
  `dest_ip` VARCHAR(128) NULL DEFAULT NULL,
  `success` TINYINT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) COMMENT 'Holds meatadata about an experiment';

-- ---
-- Table 'pcap_data'
-- Holds pcap data for each measuement
-- ---

DROP TABLE IF EXISTS `pcap_data`;

CREATE TABLE `pcap_data` (
  `id` INTEGER NULL AUTO_INCREMENT DEFAULT NULL,
  `id_Experiments` INTEGER NULL DEFAULT NULL,
  `Measurement Type` VARCHAR(64) NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) COMMENT 'Holds pcap data for each measuement';

-- ---
-- Foreign Keys
-- ---

ALTER TABLE `CommonData` ADD FOREIGN KEY (id_Experiments) REFERENCES `Experiments` (`id`);
ALTER TABLE `DiscriminationResults` ADD FOREIGN KEY (id_Experiments) REFERENCES `Experiments` (`id`);
ALTER TABLE `BaseResults` ADD FOREIGN KEY (id_Experiments) REFERENCES `Experiments` (`id`);
ALTER TABLE `Metadata` ADD FOREIGN KEY (id_Experiments) REFERENCES `Experiments` (`id`);
ALTER TABLE `pcap_data` ADD FOREIGN KEY (id_Experiments) REFERENCES `Experiments` (`id`);

-- ---
-- Table Properties
-- ---

-- ALTER TABLE `CommonData` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
-- ALTER TABLE `DiscriminationResults` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
-- ALTER TABLE `BaseResults` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
-- ALTER TABLE `Experiments` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
-- ALTER TABLE `Metadata` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
-- ALTER TABLE `pcap_data` ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- ---
-- Test Data
-- ---

-- INSERT INTO `CommonData` (`id`,`id_Experiments`,`protocol`,`num_packets`,`num_tail`,`packet_size`) VALUES
-- ('','','','','','');
-- INSERT INTO `DiscriminationResults` (`id`,`id_Experiments`,`discrimination_time`,`discrimination_losses_str`,`disc_src_port`,`disc_dest_port`,`disc_tos`,`disc_id`,`disc_frag_off`,`disc_ttl`,`disc_filename`,`id_pcap_data`) VALUES
-- ('','','','','','','','','','','','');
-- INSERT INTO `BaseResults` (`id`,`id_Experiments`,`base_time`,`base_losses_str`,`base_src_port`,`base_dest_port`,`base_tos`,`base_id`,`base_ttl`,`base_frag_off`,`base_filename`,`id_pcap_data`) VALUES
-- ('','','','','','','','','','','','');
-- INSERT INTO `Experiments` (`id`) VALUES
-- ('');
-- INSERT INTO `Metadata` (`id`,`id_Experiments`,`Project`,`test_name`,`test_date`,`command`,`host_ip`,`dest_ip`,`success`) VALUES
-- ('','','','','','','','','');
-- INSERT INTO `pcap_data` (`id`,`id_Experiments`,`Measurement Type`) VALUES
-- ('','','');