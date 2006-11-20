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

			$perc=$torrent[bytes_downloaded]*100/$torrent[total_bytes];
			echo "<td><a href=\"torrent.php?id=$a\" >$torrent[torrent_name]</a></td>";
				switch ($torrent[status]) {
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
		$downspeed=$globalinfo[download_speed]/1024;
		echo "<td>Down: $downspeed kb/s</td>";
		$upspeed=$globalinfo[upload_speed]/1024;
		echo "<td>Up: $upspeed kb/s</td>";
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


</body>
</html>

