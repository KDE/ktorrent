<?php
$stats=downloadStatus();
$num_torrent=$_REQUEST['torrent'];

function cut_name_if_long($string)
{
	if(strlen($string)>30) return substr($string, 0, 30).'...';
	else return $string;
}

function get_file_status_name($status_id)
{
	$table = array(
	60 => 'PREVIEW_PRIORITY',
	50 => 'Download First',
	40 => 'Download Normally',
	30 => 'Download Last',
	20 => 'Only Seed',
	10 => 'Do Not Download'
	);
	if (array_key_exists($status_id, $table)) return $table[$status_id];
	else return 'Not supported file status';
}

function generate_file_prior_button_code($img, $alt, $href='')
{
	$img = '<img src="'.htmlspecialchars($img).'" alt="'.htmlspecialchars($alt).'" />';
	if (empty($href)) return $img;
	else return '<a href="'.htmlspecialchars($href).'">'.$img.'</a>';
}

$display_name=cut_name_if_long($stats[$num_torrent]['torrent_name']);

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
		foreach($stats[$num_torrent]['files'] as $id => $file)
		{
			echo "\t\t".'<tr>'."\n\t\t\t";
			echo '<td class="actions">';
			echo generate_file_prior_button_code('/high_priority.png', 'High Priority', $file['status']==50?'':"details.php?file_hp=$num_torrent-$id&torrent=$num_torrent");
			echo generate_file_prior_button_code('/normal_priority.png', 'Normal Priority', $file['status']==40?'':"details.php?file_np=$num_torrent-$id&torrent=$num_torrent");
			echo generate_file_prior_button_code('/low_priority.png', 'Low Priority', $file['status']==30?'':"details.php?file_lp=$num_torrent-$id&torrent=$num_torrent");
			echo generate_file_prior_button_code('/only_seed.png', 'Stop downloading (Only Seed Priority)', ($file['status']==20||$file['status']==10)?'':"details.php?file_stop=$num_torrent-$id&torrent=$num_torrent");
			echo '</td>';
			echo '<td>'.htmlspecialchars(cut_name_if_long($file['name'])).'</td>';
			echo '<td>'.get_file_status_name($file['status']).'</td>';
			echo '<td style="text-align:right;">'.$file['size'].'</td>';
			echo '<td style="text-align:right;">'.round($file['perc_done'], 2).' %</td>';
			echo "\n\t\t".'</tr>'."\n";
		}
		?>
		</table>
	</div>
	<div id="footer">&#169; 2006 WebInterface KTorrent plugin</div>
</body>
</html>
