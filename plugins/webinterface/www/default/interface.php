<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
   "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html>
<head>
    <title>KTorrent WebInterface</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <link rel="stylesheet" type="text/css" href="style.css" />
</head>
<body>
<table width="100%">
  <tbody>
    <tr>
      <td></td>
      <td align="center"><a href="interface.php"><img src="ktorrentwebinterfacelogo.png" width="340" height="150" alt="KTorrent WebInterface" /></a></td>
    </tr>
    <tr>
      <td>
      <table width="100%">

<?php
		$globalinfo=globalInfo();
		echo "<form action=\"interface.php\" method=\"get\">";
		echo "<tr><td><input type=\"text\" name=\"maximum_upload_rate\" value=\"$globalinfo[max_upload_speed]\" /> UploadSpeed</td></tr>";
		echo "<tr><td><input type=\"text\" name=\"maximum_download_rate\" value=\"$globalinfo[max_download_speed]\" /> DownloadSpeed</td></tr>";
		echo "<tr><td><input type=\"text\" name=\"maximum_downloads\" value=\"$globalinfo[max_downloads]\" /> MaximumDownload</td></tr>";
		echo "<tr><td><input type=\"submit\" /></td></tr>";
		echo "</form>";
?>
		<form action="interface.php" method="get">
		<tr><td><input type="text" name="load_torrent" />
		<input type="submit" value="Load Torrent" /></td></tr>
		</form>

		<tr><td><form action="interface.php" method="get">
		<input type="submit" name="stopall" value="Stop All" /></td></tr>
		</form>

		<form action="interface.php" method="get">
		<tr><td><input type="submit" name="startall" value="Start All" /></td></tr>
		</form>
	</table>
      </td>
      <td>
		<table width="100%">
			<tr>
				<td align="left"><strong>Actions</strong></td>
				<td align="left"><strong>File</strong></td>
				<td align="left"><strong>Status</strong></td>
				<td align="left"><strong>Downloaded</strong></td>
				<td align="left"><strong>Size</strong></td>
				<td align="left"><strong>Uploaded</strong></td>
				<td align="left"><strong>Down Speed</strong></td>
				<td align="left"><strong>Up Speed</strong></td>
				<td align="left"><strong>Peers</strong></td>
				<td align="left"><strong>Complete</strong></td>
			</tr>
				<?php
				$stats=downloadStatus();
				$a = 0;
				foreach ($stats as $torrent) {
				$perc = round(100.0 - ($torrent[bytes_left_to_download] / $torrent[total_bytes_to_download]) * 100.0, 2);
				echo "<tr>";
					echo "<td><a href=\"interface.php?stop=$a\" title=\"STOP\"><img src=\"/stop.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\"></a>";
					echo "<a href=\"interface.php?start=$a\" title=\"START\"><img src=\"/start.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\"></a>";
					echo "<a href=\"interface.php?remove=$a\" title=\"REMOVE\"><img src=\"/remove.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\"></a></td>";
					echo "<td>$torrent[torrent_name]</td>";
					switch ($torrent[status]) {
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
					echo "<td>$torrent[bytes_downloaded]</td>";
					echo "<td>$torrent[total_bytes]</td>";
					echo "<td>$torrent[bytes_uploaded]</td>";
					echo "<td>$torrent[download_rate]</td>";
					echo "<td>$torrent[upload_rate]</td>";
					echo "<td>$torrent[num_peers]</td>";
					echo "<td>$perc %</td>";
				echo"</tr>";
				$a++;
				}
			?>
		</table>
	</td>
    </tr>
  </tbody>
</table>
</body>
</html>
