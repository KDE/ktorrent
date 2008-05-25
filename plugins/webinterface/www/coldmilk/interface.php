<?php
        $refresh_rate = 5;
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
	<title>ktorrent web interface</title>
	<link rel="stylesheet" href="style.css" type="text/css" />
	<meta name="GENERATOR" content="Quanta Plus" />
	<meta http-equiv="Content-Type" content="text/html"/>
	<script type="text/javascript" src="page_update.js"></script>
	<script type="text/javascript" src="interface.js"></script>

</head>
<body onload="update_interval(<?php echo $refresh_rate; ?>);">


<div id="header">
	<div id="logout">
		<img src="icons/16x16/edit_user.png" alt="logout" /> <a href="login.html">Sign out</a>
	</div>

	<a href="interface.php">
		<img src="icon.png" alt="reload" title="reload"
			id="header_icon" />
	</a>

	<ul>
		<li>
			<img src="icons/32x32/folder1.png" alt="icon" /> 
			<a href="javascript:show('torrent_list');">Torrents</a>
		</li>
		<li>
			<img src="icons/32x32/configure.png" alt="icon" /> 
			<a href="javascript:show('preferences');">Preferences</a>
		</li>
		
		<li>
			<img src="icons/32x32/fileopen.png" alt="icon" /> 
			<a href="javascript:show('torrent_add');">Add torrent</a>
		</li>
		<li>
			<img src="icons/32x32/extender_opened.png" alt="exit" />
			<a href="javascript:show('action');">Action</a>
		</li>
	</ul>

	<div id="status_bar">
		<table id="status_bar_table"><tr><td></td></tr></table>
	</div>
</div>


<!-- Torrents -->
<div id="torrent_list">
	<table id="torrent_list_table" class="list_table">
	<tr><td></td></tr><!--let's be XHTML valid-->
	</table>

	<div id="bottom-menu">
	<ul>
		<li>
			<img src="icons/22x22/ktstart_all.png" alt="" />
			<span>
				<a href="interface.php?startall=true">Start all</a>
			</span> 
		</li>
		<li>
			<img src="icons/22x22/ktstop_all.png" alt="" />
			<span>
				<a href="interface.php?stopall=true">Stop all</a>
			</span> 
		</li>
	</ul>
	
	</div>

</div>
<!-- end torrents -->


<!-- Torrent's details -->
<div id="torrents_details" style="display : none;">
	<table id="torrents_details_files" class="list_table">
	<tr><td></td></tr><!--let's be XHTML valid-->
	</table>
</div>
<!-- end torrent's details -->


<!-- Preferences -->
<div id="preferences" style="display : none;">
	<h2>Preferences</h2>
	<form action="interface.php" method="get">
	<div class="simple_form">
		<img src="icons/64x64/down.png" alt="" />

		<h2>Downloads</h2>

		<?php $globalinfo = globalinfo() ?>
		<div class="item" style="margin-top : 0em;">
		Upload speed:
		<div class="option">
			<input type="text" name="maximum_upload_rate" 
				value="<?php echo $globalinfo['max_upload_speed']; ?>" />
		</div>
		</div>

		<div class="item">
		Download speed:
		<div class="option">
			<input type="text" name="maximum_download_rate" 
				value="<?php echo $globalinfo['max_download_speed']; ?>" />
		</div>
		</div>

		<div class="item">
		Max downloads:
		<div class="option">
			<input type="text" name="maximum_downloads" 
				value="<?php echo $globalinfo['max_downloads']; ?>" />
		</div>
		</div>

		<div class="item">
		Max seeds
		<div class="option">
			<div style="display : inline;">
				<input type="text" name="maximum_seeds" 
					value="<?php echo $globalinfo['max_seeds']; ?>" />
			</div>
		</div>
		</div>
	</div>

	<div class="simple_form" style="margin-top : 1em;" >
		<img src="icons/64x64/looknfeel.png" alt="" />

		<h2>Web interface</h2>

		<div class="hints">
			Note: Disabled for now. If you insist, change $refresh_rate in the file interface.php
		</div>

		<div class="item">
		<?php
		$refresh_options = array(
			'2'   => '2 seconds',
			'3'   => '3 seconds',
			'5'   => '5 seconds',
			'10'  => '10 seconds',
			'30'  => '30 seconds',
			'0'   => 'never'
		);
		echo 'Update rate:';
		echo '<div class="option">';
		echo '<select name="refresh_rate" disabled="disabled">';
		foreach(array_keys($refresh_options) as $value) {
			echo '<option value="'.$value.'"';
				if ($refresh_rate == $value) {
					echo ' selected="selected"';
				}
				echo '>'.$refresh_options[$value].'</option>';
		}
		echo '</select>';
		echo '</div>';
		?>
		</div>

	</div>
	
	<div style="margin-top : 1em; float : left; clear : both;">
		<input type="submit" value="Submit preferences" class="buttons"/>
	</div>
	</form>
</div>
<!-- end preferences -->


<!-- Add Torrent -->
<div id="torrent_add" style="display : none;">
	<h2>Add a torrent</h2>

	<div class="simple_form">
		<img src="icons/64x64/folder1_man.png" alt="" />
		<h3>Load a torrent</h3>

		<form action="interface.php" method="get">
		<div class="item">
			URL:
			<div class="option">
			<input type="text" name="load_torrent" style="width : 240px;" />
			<br /><span>Example: http://ktorrent.org/down/latest.torrent</span>
			
			<div style="margin-top : 1em;">
				<input type="submit" value="Load Torrent" />
			</div>
			</div>
		</div>
		</form>

		<h3 style="margin-top : 6em;">Upload a torrent</h3>

		<form action="interface.php" method="post" enctype="multipart/form-data">
		<div class="item" style="min-height : 5em;">
			File path:
			<div class="option">
			<div style="display : inline;">
				<input type="file" name="load_torrent" style="width:240px;" />
			</div>

			<div style="margin-top : 1em;">
				<input type="submit" name="Upload Torrent" value="Upload Torrent" />
			</div>
			</div>
		</div>
		</form>
	</div>
</div>
<!-- end add torrent -->


<!-- Action -->
<div id="action" style="display : none;">
	<h2 style="margin-top : 0; padding-top : 0;">Actions</h2>
	<ul>
		<li>
			<img src="icons/48x48/switchuser.png" alt="sign out" />
			<span style="margin-left : 52px;">
			<a href="login.html">Sign out</a></span>

	
		</li>

		<li>
			<img src="icons/48x48/exit.png" alt="quit" />
			<span style="margin-left : 52px;">
				<a href="shutdown.php?quit=quit" onclick="return validate('quit_program')">
					Quit program
				</a></span>
		</li>

	</ul>	

</div>
<!-- end action -->


</body>
</html>
