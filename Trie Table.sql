

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";



--
-- Table structure for table `trie`
--

CREATE TABLE `trie` (
  `WORD` varchar(50) DEFAULT NULL,
  `SONG` int(11) NOT NULL DEFAULT '0',
  `ARTIST` int(11) NOT NULL DEFAULT '0',
  `TYPE` int(11) NOT NULL DEFAULT '0',
  `ALBUM` int(11) NOT NULL DEFAULT '0',
  `BAND` int(11) NOT NULL DEFAULT '0',
  `PREF` float NOT NULL DEFAULT '0',
  `UPDATED` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='BACKUP FOR TRIE';

-- --------------------------------------------------------

