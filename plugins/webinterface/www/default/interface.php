<?php
$globalinfo=globalInfo();
$stats=downloadStatus();

function get_torrent_status_name($status_id)
{
	$table = array(
	0 => 'Not Started',
	1 => 'Seeding Complete',
	2 => 'Download Complete',
	3 => 'Seeding',
	4 => 'Downloading',
	5 => 'Stalled',
	6 => 'Stopped',
	7 => 'Allocating Diskspace',
	8 => 'Error',
	9 => 'Queued',
	10 => 'Checking Data'
	);
	if (array_key_exists($status_id, $table)) return $table[$status_id];
	else return 'Not supported Status';
}

function generate_button_code($img, $alt, $href='')
{
	$img = '<img src="'.htmlspecialchars($img).'" alt="'.htmlspecialchars($alt).'" />';
	if (empty($href)) return $img;
	else return '<a href="'.htmlspecialchars($href).'">'.$img.'</a>';
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<style type="text/css" media="all">
	@import "stylen.css";
</style>
<meta http-equiv="Content-Type" content="text/html" />
<link rel="icon" href="favicon.ico" type="image/x-icon" />
<link rel="shortcut icon" href="favicon.ico" type="image/x-icon" />
<title><?php echo '(D:'.$globalinfo['download_speed'].') (U:'.$globalinfo['upload_speed'].') KTorrent'; ?></title>
<script type="text/javascript">
	function validate()
	{
		msg = "Are you absolutely sure that you want to remove this torrent?";
		return confirm(msg);
	}
	function validate_shutdown()
	{
		msg = "Are you absolutely sure that you want to shutdown KTorrent?";
		return confirm(msg);
	}
</script>
</head>
<body>
	<div id="top_bar">WebInterface KTorrent plugin</div>
	<div id="icon"><img src="icon.png" alt="" /></div>
	<div id="header">
		<strong>KTorrent WebInterface</strong>
		<br />
        	<small>BitTorrent client for KDE</small>
	</div>
	<ul id="menu">
		<li><a href="shutdown.php?quit=quit" class="shutdown" title="Shutdown KTorrent" onclick="return validate_shutdown()">Shutdown</a></li>
		<li><a href="interface.php" title="REFRESH">Refresh</a></li>
		<li><a href="login.html" title="LOGOUT">Logout</a></li>
	</ul>
	<div id="sidebar">
		<div class="box">
			<h2>Torrent control</h2>
			<form action="interface.php" method="get" style="text-align: center;">
			<input type="submit" name="startall" value="Start All" />
			</form>
			<hr />
			<form action="interface.php" method="get" style="text-align: center;">
			<input type="submit" name="stopall" value="Stop All" />
			</form>
		</div>
		<div class="box">
			<h2>Settings</h2>
			<form action="interface.php" method="get">
				<label>Upload speed <input type="text" name="maximum_upload_rate" value="<?php echo $globalinfo['max_upload_speed']; ?>" class="settingsInput" /></label>
				<label>Download speed <input type="text" name="maximum_download_rate" value="<?php echo $globalinfo['max_download_speed']; ?>" class="settingsInput" /></label>
				<label>Maximum downloads <input type="text" name="maximum_downloads" value="<?php echo $globalinfo['max_downloads']; ?>" class="settingsInput" /></label>
				<label>Maximum seeds <input type="text" name="maximum_seeds" value="<?php echo $globalinfo['max_seeds']; ?>" class="settingsInput" /></label>
                        	<input type="submit" value="Submit settings" />
			</form>
		</div>
		<div class="box">
			<h2>Load torrents</h2>
			<form action="interface.php" method="get">
			<label class="wide">Torrent URL: <input type="text" name="load_torrent" /></label>
			<input type="submit" value="Load Torrent" />
			</form>
			<hr />
			<form method="post" enctype="multipart/form-data" action="interface.php">
			<label class="wide">Local File:<input type="file" name="load_torrent" /></label>
			<input type="submit" name="Upload Torrent" value="Upload Torrent" />
			</form>
		</div>
	</div>
	<div id="content">
		<table>
		<tr>
			<th>Actions</th>
			<th>File</th>
			<th>Status</th>
			<th>Downloaded</th>
			<th>Size</th>
			<th>Uploaded</th>
			<th>Down Speed</th>
			<th>Up Speed</th>
			<th>Peers</th>
			<th>Complete</th>
		</tr>
<?php
		$a = 0;
		foreach ($stats as $torrent) {
			echo "\t\t".'<tr>'."\n\t\t\t";

			$torrent_name = str_replace("'", "\'", $torrent['torrent_name']);
			if($torrent['total_bytes_to_download']!=0) $perc = round(100.0 - ($torrent['bytes_left_to_download'] / $torrent['total_bytes_to_download']) * 100.0, 2);
			else $perc = 0;
			if(strlen($torrent['torrent_name'])>30) $display_name=substr($torrent['torrent_name'], 0, 30)." ...";
			else $display_name=$torrent['torrent_name'];
			if ($torrent['num_files']>1) $file_td_content = '<a href="details.php?torrent='.$a.'">'.$display_name.'</a>';
			else $file_td_content = $display_name;

			echo '<td class="actions">';
			echo generate_button_code('/stop.png', 'stop', ($torrent['running'])?'interface.php?stop='.$a:'');
			echo generate_button_code('/start.png', 'start', ($torrent['running'])?'':'interface.php?start='.$a);
			echo '<a href="interface.php?remove='.$a.'" onclick="return validate()"><img src="/remove.png" alt="remove" /></a>';
			echo '</td>';
			echo "<td style=\"text-align:left;\" onmouseover=\"this.T_TITLE='$torrent_name';return escape('Download speed:&lt;strong&gt;{$torrent['download_rate']}&lt;/strong&gt;&lt;br /&gt; Upload speed:&lt;strong&gt;{$torrent['upload_rate']}&lt;/strong&gt;&lt;/td&gt;')\">$file_td_content</td>";
			echo '<td>'.get_torrent_status_name($torrent['status']).'</td>';
			echo '<td style="text-align:right;">'.$torrent['bytes_downloaded'].'</td>';
			echo '<td style="text-align:right; padding-left:8px;">'.$torrent['total_bytes'].'</td>';
			echo '<td style="text-align:right; padding-left:8px;">'.$torrent['bytes_uploaded'].'</td>';
			echo '<td style="text-align:right;">'.$torrent['download_rate'].'</td>';
			echo '<td style="text-align:right;">'.$torrent['upload_rate'].'</td>';
			echo '<td>'.$torrent['num_peers'].'</td>';
			echo '<td style="text-align:right;">'.$perc.'% </td>';
			echo "\n\t\t".'</tr>'."\n";
		$a++;
		}
		?>
		</table>
	</div>
	<div id="footer">&#169; 2006 WebInterface KTorrent plugin</div>
	<script type="text/javascript" src="wz_tooltip.js"></script>
</body>
</html>
