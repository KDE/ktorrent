<html>
<head>
<title>KTorrent WebInterface</title>
</head>
<body>
<table  width="100%">
  <tbody>
    <tr>
      <td align="center"><IMG src="ktorrentwebinterfacelogo.png" width="340" height="150" align="top" border="0"></td>
      
      <td><strong>ktorrent</strong>->transfers</td>
      
      <td><td><a href="interface.php" >refresh</a></td>
    </tr>
      </tbody>
</table>
<hr>
<table  width="100%">
  <tbody>
    	<?php
		$stats=downloadStatus();
		$a = 0;
		foreach ($stats as $torrent) {
			echo "<tr>";
			$perc = round(100.0 - ($torrent['bytes_left_to_download'] / $torrent['total_bytes_to_download']) * 100.0, 2);
			echo "<td><a href=\"torrent.php?id=$a\" >{$torrent['torrent_name']}</a></td>";
				switch ($torrent['status']) {
						case 0:
   							echo "<td>Not Started</td>";
							break;
						case 1:
  							echo "<td>Seeding Complete</td>";
   							break;
						case 2:
							echo "<td>Download Complete</td>";
							break;
						case 3:
							echo "<td>Seeding</td>";
							break;
						case 4:
							echo "<td>Downloading</td>";
							break;
						case 5:
							echo "<td>Stalled</td>";
							break;
						case 6:
							echo "<td>Stopped</td>";
							break;
						case 7:
							echo "<td>Allocating Diskspace</td>";
							break;
						case 8:
							echo "<td>Error</td>";
							break;
						case 9:
							echo "<td>Queued</td>";
							break;
						case 10:
							echo "<td>Checking Data</td>";
							break;
						default:
							echo "<td>Not supported Status</td>";
				}
				echo "<td>$perc%</td>";
      		$a=$a+1;	
		echo "</tr>";
		}
      	?>
    
    <tr>
    	<td>&nbsp;</td>
	<td><hr></td>
	<td>&nbsp;</td>
    </tr>

    <tr>
    	<?php
		$globalinfo=globalInfo();
		echo "<td><strong>Speed</strong></td>";
		echo "<td>Down: {$globalinfo['download_speed']}</td>";
		echo "<td>Up: {$globalinfo['upload_speed']}</td>";
	?>
    </tr>
    <tr>
    	<td>&nbsp;</td>
	<td><hr></td>
	<td>&nbsp;</td>
    </tr>
    <tr>
    	<td><a href="interface.php?startall=startall" ><strong>Start All</strong></a></td>
	<td>&nbsp;</td>
	<td><a href="interface.php?stopall=stopall" ><strong>Stop All</strong></a></td>
    </tr>
    <tr>
    	<td>&nbsp;</td>
	<td><a href="settings.php" ><strong>Settings</strong></a></td>
	<td>&nbsp;</td>	
    </tr>

  </tbody>
</table>
<FORM method="GET">
<INPUT type="text" name="load_torrent">
<INPUT type="submit" name="Load torrent" value="Load torrent"></tr>
</FORM>
<FORM method="post" enctype="multipart/form-data" action="interface.php">
Local File:<INPUT type="file" name="load_torrent">
<INPUT type="submit" name="Upload Torrent" value="Upload Torrent"></tr>
</FORM>

</body>
</html>

