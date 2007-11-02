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
    <style type="text/css" media="all">@import "stylen.css";

	.settingsInput
	{
		width:50px;
	}

    </style>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <link rel="icon" href="favicon.ico" type="image/x-icon"/>
    <link rel="shortcut icon" href="favicon.ico" type="image/x-icon"/>
    <title><?php echo 'KTorrent: Details for '.$display_name; ?></title>

  </head>
  <body>
    <div id="page-container">
      <div id="top_bar">
        WebInterface KTorrent plugin
      </div>
      <div id="icon">
        <img src="icon.png" alt="KT Icon"/>
      </div>
      <div id="header">
        <div id="head-content">
          <strong>KTorrent WebInterface</strong>
          <br />
          <font size="-1">BitTorrent client for KDE</font>
        </div>
		<div id="menu">
				<div style="padding-left:130px;">

					<dl>
					<?php
							echo "<dt><a href=\"details.php?torrent=$num_torrent\" title=\"REFRESH\">Refresh</a></dt>";
					?>
							<dt><a href="interface.php" title="BACK">Back</a></dt>
							<dt><a href="login.html" title="LOGOUT">Logout</a></dt>
					</dl>
				</div>
			</div>
      </div>
      <div id="content">
        <table>
          <tr>
            <td align="left" style="padding:2px;">
              <strong>Actions</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>File</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Status</strong>
            </td>
            <td align="right" style="padding:2px;">
              <strong>Size</strong>
            </td>
            <td align="left" style="padding:2px;">
              <strong>Complete</strong>
            </td>
          </tr>

	  <?php
		$files_to_display=$stats[$num_torrent]['num_files'];
		$display_files = $stats[$num_torrent]['files'];

		for ($i = 0; $i < $files_to_display; $i++)
		{
			$file_pos="file_$i";
			$size_pos="size_$i";
			$perc_pos="perc_done_$i";
			$status_pos="status_$i";

			echo "<tr>";
			echo "<td><a href=\"details.php?file_hp=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"High Priority\"><img src=\"/high_priority.png\" name=\"stop\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\" alt=\"/\\\" /></a>";
			echo "<a href=\"details.php?file_np=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Normal Priority\"><img src=\"/normal_priority.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\" alt=\"-\" /></a>";
			echo "<a href=\"details.php?file_lp=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Low Priority\"><img src=\"/low_priority.png\" name=\"start\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\" alt=\"\\/\" /></a>";
			echo "<a href=\"details.php?file_dnd=$num_torrent-$i&amp;torrent=$num_torrent\" title=\"Only Seed Priority\"><img src=\"/dnd.png\" name=\"remove\" width=\"16\" height=\"16\" border=\"0\" style=\"padding:2px;\" alt=\"X\" /></a></td>";

			if(strlen($display_files[$file_pos])>30)
				$file_display=substr($display_files[$file_pos], 0, 30)."...";
			else
				$file_display=$display_files[$file_pos];
					
			$perc_display=round($display_files[$perc_pos], 2);

			echo "<td>$file_display</td>";
			echo "<td>$display_files[$status_pos]</td>";
			echo "<td style=\"text-align:right;\">$display_files[$size_pos]</td>";
			echo "<td style=\"text-align:right;\">$perc_display %</td>";
			echo "</tr>";
		}
	  ?>
	 </table>
	
	</div>
      <div id="footer">
        <table width="100%" align="center">
          <tr>
            <td>
            </td>
            <td>
               &#169; 2006 WebInterface KTorrent plugin
            </td>
            <td>
              <a href="login.html" title="LOGOUT">Logout</a> | <a href="shutdown.php?quit=quit" title="Shutdown KTorrent">Shutdown KTorrent</a>
            </td>
          </tr>
        </table>
      </div>
     </div>
  </body>
</html>
