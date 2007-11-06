<?php
	$stats=downloadStatus();
	$num_torrent=$_REQUEST['torrent'];
	if(strlen($stats[$num_torrent]['torrent_name'])>30)
		$display_name=substr($stats[$num_torrent]['torrent_name'], 0, 30)."...";
	else
		$display_name=$stats[$num_torrent]['torrent_name'];
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
<title><?php echo 'KTorrent: Details for '.$display_name; ?></title>
<script type="text/javascript">
	function validate()
	{
		msg = "Are you absolutely sure that you want remove this torrent?";
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
		<li><a href="interface.php" title="BACK">Back</a></li>
		<li><a href="login.html" title="LOGOUT">Logout</a></li>
	</ul>
	<div id="content">
		<table>
		<tr>
			<th>Actions</th>
			<th>File</th>
			<th>Status</th>
			<th>Size</th>
			<th>Complete</th>
		</tr>
		<?php
		$files_to_display=$stats[$num_torrent]['num_files'];
		$display_files = $stats[$num_torrent]['files'];

		for ($i = 0; $i < $files_to_display; $i++)
		{
			$file_pos='file_'.$i;
			$size_pos='size_'.$i;
			$perc_pos='perc_done_'.$i;
			$status_pos='status_'.$i;

			echo '<tr>';
			echo "<td class=\"actions\"><a href=\"details.php?file_hp=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"High Priority\"><img src=\"/high_priority.png\" name=\"stop\" alt=\"/\\\" /></a>";
			echo "<a href=\"details.php?file_np=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Normal Priority\"><img src=\"/normal_priority.png\" name=\"start\" alt=\"-\" /></a>";
			echo "<a href=\"details.php?file_lp=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Low Priority\"><img src=\"/low_priority.png\" name=\"start\" alt=\"\\/\" /></a>";
			echo "<a href=\"details.php?file_dnd=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Only Seed Priority\"><img src=\"/dnd.png\" name=\"remove\" alt=\"X\" /></a></td>";

			if(strlen($display_files[$file_pos])>30)
				$file_display=substr($display_files[$file_pos], 0, 30)."...";
			else
				$file_display=$display_files[$file_pos];
					
			$perc_display=round($display_files[$perc_pos], 2);

			echo '<td>'.$file_display.'</td>';
			echo '<td>'.$display_files[$status_pos].'</td>';
			echo '<td style="text-align:right;">'.$display_files[$size_pos].'</td>';
			echo '<td style="text-align:right;">'.$perc_display.' %</td>';
			echo '</tr>';
		}
		?>
		</table>
	</div>
	<div id="footer">&#169; 2006 WebInterface KTorrent plugin</div>
	<script type="text/javascript" src="wz_tooltip.js"></script>
</body>
</html>
