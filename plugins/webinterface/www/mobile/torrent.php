<html>
<head>
<title>KTorrent WebInterface</title>
</head>
<body>
<table  width="100%">
  <tbody>
    <tr>
      <td align="center"><IMG src="ktorrentwebinterfacelogo.png" width="340" height="150" align="top" border="0"></td>
      <?php
      	$stats=downloadStatus();
      	$t=$stats[$_REQUEST['id']];
      	echo "<td><strong>ktorrent-><a href=\"interface.php\">transfers</a></strong>->{$t['torrent_name']}</td>";
      	echo "<td><a href=\"torrent.php?id={$_REQUEST['id']}\" >refresh</a></td>";
      ?>
    </tr>
  </tbody>
</table>
<table  width="100%">
  <tbody>
    <tr>
      <?php
		echo "<td><a href=\"torrent.php?stop={$_REQUEST['id']}&id={$_REQUEST['id']}\" title=\"STOP\"><img src=\"/stop.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\"></a></td>";
		echo "<td><a href=\"torrent.php?start={$_REQUEST['id']}&id={$_REQUEST['id']}\" title=\"START\"><img src=\"/start.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\"></a></td>";
		echo "<td><a href=\"interface.php?remove={$_REQUEST['id']}\" title=\"REMOVE\"><img src=\"/remove.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\"></a></td>";
      ?>
    </tr>
  </tbody>
</table>
<table  width="100%">
  <tbody>
      <?php
      	echo "<tr>";
       	echo "<td><strong>Status: </strong></td>";
      	switch ($t['status']) {
		case 0:
   			echo "<td>NOT_STARTED</td>";
			break;
		case 1:
  			echo "<td>SEEDING_COMPLETE</td>";
   			break;
		case 2:
			echo "<td>DOWNLOAD_COMPLETE</td>";
			break;
		case 3:
			echo "<td>SEEDING</td>";
			break;
		case 4:
			echo "<td>DOWNLOADING</td>";
			break;
		case 5:
			echo "<td>STALLED</td>";
			break;
		case 6:
			echo "<td>STOPPED</td>";
			break;
		case 7:
			echo "<td>ALLOCATING_DISKSPACE</td>";
			break;
		case 8:
			echo "<td>ERROR</td>";
			break;
		case 9:
			echo "<td>QUEUED</td>";
			break;
		case 10:
			echo "<td>CHECKING_DATA</td>";
			break;
		default:
			echo "<td>Not supported Status</td>";
		}
	echo "</tr>";
	echo "<tr>";
	echo "<td><strong>Down speed: </strong></td>";
	echo "<td>{$t['download_rate']}</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<td><strong>Up speed: </strong></td>";
	echo "<td>{$t['upload_rate']}</td>";
	echo "</tr>";
	echo "<tr>";
	echo "<td><strong>Complete: </strong></td>";
	$perc = round(100.0 - ($t['bytes_left_to_download'] / $t['total_bytes_to_download']) * 100.0, 2);
	echo "<td>$perc %</td>";
	echo "</tr>";
      ?>
  </tbody>
</table>
</body>
</html>

