<?php

  /***************************************************************************
 *   Copyright (C) 2007 by Dagur Valberg Johannsson                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

/**
  * Simple REST interface.
  */

$rest_commands = array(
	"global_status",
	"download_status",
	"torrents_details"
);

if (!array_keys($_REQUEST)) {
	header("Content-Type: text/html");
	print "<html><body>
			Usage: rest.php?command<br />
			Available commands: ";
	foreach ($rest_commands as $command) 
		print "$command ";
	
	print "</body></html>";
}

else {
	header("Content-Type: text/xml");
	$rest = new RestInterface();
	foreach($_REQUEST as $command=>$arg)
	{
		if (in_array($command, $rest_commands))
		print $rest->$command($arg);
		else
		print "Unknown command " . htmlentities($command) . "<br />";	
		
	}
}

// classes

class RestInterface {
	public function global_status() {
		$info = globalinfo();
		$common = new Common();

		$down_speed = $info['download_speed'];
		$up_speed = $info['upload_speed'];
		
		$down_total 
			= $common->bytes_to_readable($info['bytes_downloaded']);
		$up_total
			= $common->bytes_to_readable($info['bytes_uploaded']);
		
		$dht        = $info['dht_support'] ? "on" : "off";
		$encryption = $info['use_encryption'] ? "on" : "off";
		
		$xml = new KTorrentXML('status_bar');
		
		$elements = array(
			$xml->new_element('download_speed'  , $down_speed),
			$xml->new_element('upload_speed'    , $up_speed),
			$xml->new_element('downloaded_total', $down_total),
			$xml->new_element('uploaded_total',   $up_total),
		
			$xml->new_element('dht', null, array('status' => $dht)),
			$xml->new_element('encryption', null, array('status' => $encryption)),
		);
		
		foreach($elements as $element) {
			$xml->append_to_root($element);
		}
		
		return $xml->saveXML();
	
	}
	
	public function download_status() {
		$download_status = downloadstatus();
		$xml = new KTorrentXML('download_status');
		foreach($download_status as $torrent) {
			$torrent_xml = $xml->new_element('torrent');
			$xml->append_to_root($torrent_xml);

//			foreach(array_keys($torrent) as $key) {
//				$torrent_xml->appendChild(
//				$xml->new_element("raw_$key", $torrent[$key]));
//			}
			
			$status = $torrent['status'];
			$done = $torrent['bytes_downloaded'];
			$total_bytes = $torrent['total_bytes_to_download'];
			$bytes_left  = $torrent['bytes_left_to_download'];
			$elements = array(
		
				$xml->new_element('name',
					$this->_clean_name($torrent['torrent_name'])),

				$xml->new_element('status',
					$this->_torrent_status($status), array('id' => $status)),

				$xml->new_element('running', $torrent['running']),
				$xml->new_element('download_rate', $torrent['download_rate']." down"),
				$xml->new_element('upload_rate', $torrent['upload_rate']." up"),
				$xml->new_element('size', $torrent['total_bytes']),
				$xml->new_element('peers', $torrent['num_peers']),
				$xml->new_element('uploaded', $torrent['bytes_uploaded']." uploaded"),

					
				$xml->new_element('downloaded', 
					"$done downloaded",
					array('percent' => $this->_get_percent_done($total_bytes, $bytes_left))),
			);
			
			foreach($elements as $element) {
				$torrent_xml->appendChild($element);
			}
		}
		
		return $xml->saveXML();
	
	}

	public function torrents_details($torrent_id) {
		$xml = new KTorrentXML('torrents_details', null, array('id'=>$torrent_id));
		$download_status = downloadstatus();
		if (isset($download_status[$torrent_id]))
		foreach($download_status[$torrent_id]['files'] as $id=>$info)
		{
			$file_xml = $xml->new_element('file', '', array('id'=>$id));
			$xml->append_to_root($file_xml);
			foreach($info as $key=>$val)
			$file_xml->appendChild($xml->new_element($key, $val));
		}
		return $xml->saveXML();
	}

	// Helper function for download_status
	private function _torrent_status($status_id) {
		$status = array(
			0  => "Not started",
			1  => "Seeding Complete",
			2  => "Download Complete",
			3  => "Seeding",
			4  => "Downloading",
			5  => "Stalled",
			6  => "Stopped",
			7  => "Allocating Diskspace",
			8  => "Error",
			9  => "Queued",
			10 => "Checking Data"
		);
		
		return $status[$status_id];
	}

	
	// Truncate long torrent name, and HTML escape it.
	// This is a helper function for download_status.
	private function _clean_name($name) {
		$name = str_replace("'", "\'", $name);
		if (strlen($name) > 30) {
				$name = substr($name, 0, 27);
				$name .= "...";
		}
		$name = htmlspecialchars($name);
		return $name;
	}

	// Calculate percent done.
	// Helper function for download_status	
	private function _get_percent_done($bytes_total, $bytes_left) {
		if($bytes_total) {
			$perc_done = round(100.0 - ($bytes_left / $bytes_total) * 100);
			return $perc_done;
		}
		else {
			return 0;
		}
	}
}


/** 
  * Class to build a xml tree
  */
class KTorrentXML extends DomDocument { 
	private $root_element;
	public function __construct($root, $value = null, $attributes = null) {
		parent::__construct('1.0');
		$this->root_element = $this->createElement($root);
		$this->appendChild($this->root_element);
		$this->formatOutput = true;

		if ($attributes)
		foreach($attributes as $key=>$val)
		$this->root_element->setAttribute($key, $val);
	}
	
	// Creates an element, and returns it.
	public function new_element($name, $value = null, $attributes = null) {
		$element = $this->createElement($name);
		if ($value) {
			$element->appendChild($this->createTextNode($value));
		}
		if ($attributes) {
			foreach(array_keys($attributes) as $key) {
				$element->setAttribute($key, $attributes[$key]);
			}
		}
		return $element;
	}
	
	// Append a given element to the root element of the xml file.
	public function append_to_root($element) {
		$this->root_element->appendChild($element);
	}

}

/**
  * Generic functions
  */
class Common {
	function bytes_to_readable($bytes) {
		if ($bytes < 1024) {
			return round($bytes, 2) . " bytes"; 
		}
		
		else if (($kb = ($bytes / 1024)) < 1024) {
			return round($kb, 2) . " KB";
		}
		
		else if (($mb = ($kb / 1024)) < 1024) {
			return round($mb, 2) . " MB";
		}
		
		else {
			$gb = round($mb / 1024, 2);
			return "$gb GB";
		}
	}
	
	function kb_to_readable($kbytes) {
		return Common::bytes_to_readable($kbytes * 1024);
	}
}
 
?>