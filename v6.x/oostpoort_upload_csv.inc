<?php

/* 
**  Church Administration (oostpoort) module for drupal 
**
**  Copyright (C) 2009-2010
**  =======================
**
**  Created by wplaat
**
**  For more information visit the following website.
**  Website : http://www.plaatsoft.nl 
**
**  Or send an email to the following address.
**  Email   : info@plaatsoft.nl
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, version 2.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// ##################################################################################
// Views
// ##################################################################################

function oostpoort_upload_csv() {

	// Check access for this page.
	if (!oostpoort_visit_member_admin_access()) {
		return drupal_access_denied();
	}
			
	// Breadcrumb menu
	$node["child1"] = t('ledenbestand upload');
	$page .= oostpoort_breadcrumb($node);
	
	drupal_set_title(t('Ledenbestand upload'));
	
	$page .= '<form enctype="multipart/form-data" name="OostpoortForm" method="POST" >';
	$page .= '<div class="oostpoort">';
	
	$page .= oostpoort_save_csv();
		
	$page .= '<fieldset>' ;
	$page .= '<legend>'.t('Leden bestand upload').'</legend>';	
	$page .= '<div class="form-item">';
	$page .= '  <label>'.t('Ledenbestand').':</label>';
	$page .= '  <input name="uploadedfile" type="file" size="100" onChange="submit();" />';
	$page .= '  <div class="description">';
	$page .= t('Alleen csv formaat wordt ondersteund en het bestand mag niet groter zijn dan ').ini_get('upload_max_filesize').'B';
	$page .= '  </div>';
	$page .= '</div>';
	$page .= '</fieldset>' ;  
	
	$page .= '<a href="'.url(OOSTPOORT_MENU).'">'.t('Terug').'</a>';
	
	$page .= '</div>';
	$page .= '</form>';  
		
	print theme("page", $page);
}

// ##################################################################################
// Utils
// ##################################################################################

function oostpoort_save_csv()
{
	// Fetch Image upload in a HTTP request
	if ($_FILES['uploadedfile']['tmp_name']!='') 
	{
		if ( (strstr($_FILES['uploadedfile']['name'],'.csv')) || 
		     (strstr( $_FILES['uploadedfile']['name'],'.CSV')) ) {
	
			$page .= oostpoort_process_csv($_FILES['uploadedfile']['tmp_name']);
			
		} else {
				
			$msg = t('Alleen CSV bestand formaat wordt ondersteund.');
			drupal_set_message($msg, 'error');
	
			$_FILES['uploadedfile']['name']='';
		}
	}
	return $page;
}
		
function oostpoort_process_csv($filename) {

	$line_counter=0;
	
	$address_error_counter=0;
	$address_insert_counter=0;
	$address_update_counter=0;
	
	$member_error_counter=0;
	$member_insert_counter=0;
	$member_update_counter=0;
	
	$fh = fopen($filename, 'r');
	
	// Skip this first Header line!
	$theData = fgets($fh);
	$line_counter++;
	
	// Process Data
	$theData = fgets($fh);
	while ($theData!='')
	{	
	   $theData=oostpoort_filter_data($theData);
		
		list( 
			$data["Status"],
			$data["Selectiewijk"],
			$data["Selectiesectie"],
			$data["Wijknaam"],
			$data["Geslacht"],
			$data["Aanschrijfnaam"],
			$data["VoorvoegselNaam"],
			$data["Voornamen"],
			$data["Straatnaam"],
			$data["Huisnr"],
			$data["HuisnrToev"],
			$data["HuisLetter"],
			$data["Postcode"],
			$data["Woonplaats"],
			$data["Leeftijd"],
			$data["GeboorteDatum"],
			$data["GeboortePlaats"],
			$data["Burgelijkstaat"],
			$data["KerkelijkeStaat"],
			$data["HuwelijkDatum"],
			$data["HuwelijkGezindte"],
			$data["DoopDatum"],
			$data["DoopPlaats"],
			$data["KerkGezindte"],
			$data["BelijdenisDatum"],
			$data["BelijdenisPlaats"],
			$data["HerkomstDatum"],
			$data["HerkomtPlaats"],
			$data["MutatieDatum"],
			$data["KerkRelatie"],
			$data["KerkBlad"],
			$data["KerkLidNr"],) = split(";", $theData);
	
			//echo oostpoort_show_array($data);
					
		if (($data["Huisnr"].$data["HuisLetter"]!="") && ($data["Postcode"]!="")) {
	 
			// Find address
			$AdrId = oostpoort_find_address($data);			
			if ($AdrId=="") {
			
				// Address not found, insert it!
				$AdrId = oostpoort_insert_adress($data);
				$address_insert_counter++;
			} else {
				oostpoort_update_address($data,$AdrId);
				$address_update_counter++;
			}
		} else {
			$AdrId="";
			$address_error_counter++;
		}
		
		// If validate address available update/insert member data.
		if ($AdrId!="") {
			$PrsId = oostpoort_find_member($data);
			if ($PrsId == "") {
		
				// Member not found, insert it
				$PrsId=oostpoort_insert_member($data, $AdrId);
			
				$member_insert_counter++;
		
			} else {
		
				// Member found, update it!
				oostpoort_update_member(
					$data,
					$PrsId,
					$AdrId);
		
				$member_update_counter++;
			}
			
		} else {
		
			$member_error_counter++;
		}
		
		$line_counter++;
		$theData = fgets($fh);
	}
	fclose($fh);

	$msg = t('Ledenbestand is verwerkt.');
	drupal_set_message($msg, 'status');
	
	$msg = ($line_counter-1).' '.t('leden geanalyseerd.');
	drupal_set_message($msg, 'status');
	
	$msg = $address_insert_counter.' '.t('adressen toegevoegd.');
	drupal_set_message($msg, 'status');

	$msg = $address_update_counter.' '.t('adressen bijgewerkt.');
	drupal_set_message($msg, 'status');

	$msg = $address_error_counter.' '.t('addressen niet verwerkt (data incorrect).');
	drupal_set_message($msg, 'status');

	$msg = $member_insert_counter.' '.t('personen toegevoegd.');
	drupal_set_message($msg, 'status');

	$msg = $member_update_counter.' '.t('personen bijgewerkt.');
	drupal_set_message($msg, 'status');

	$msg = $member_error_counter.' '.t('personen niet verwerkt (data incorrect).');
	drupal_set_message($msg, 'status');

	$msg .= t('Nieuw oostpoort CSV data file geladen.');
	watchdog('user', $msg);
	
	// Update drupal module variable
	variable_set("oostpoort_last_upload_date",date("d-m-Y"));
	
	return $page;
}

function oostpoort_filter_data($theData) {
	// Remove some unwanted characters and other tokens
	$theData=str_replace("Jonkvrouw ", "", $theData); 
	$theData=str_replace("Jonkheer ", "", $theData); 
	$theData=str_replace("�", "i", $theData); 
	$theData=str_replace("�", "u", $theData); 
	$theData=str_replace("�", "o", $theData); 
	$theData=str_replace("�", "e", $theData); 
	$theData=str_replace("�", "e", $theData);
	$theData=str_replace("�", "c", $theData);
	return $theData;
}

function oostpoort_find_address($data) {
	
	$HuisNr = $data["Huisnr"].$data["HuisLetter"];
	
	$sql  = 'select ';
	$sql .= '  Adr_ID as AdrId ';
	$sql .= 'from ';
	$sql .= '  oostpoort_adres ';
	$sql .= 'where ';
	$sql .= '  Adr_Postcode="'.$data["Postcode"].'" and ';
	$sql .= '  Adr_Huisnr="'.$HuisNr.'"';
 
	oostpoort_debug_sql($sql);
	$queryResult = db_query($sql);     
	$result = db_fetch_object($queryResult);
	
	$AdrId=$result->AdrId;	
	oostpoort_debug_param('AdrId',$AdrId);
	
	return $AdrId;
}
	
function oostpoort_insert_adress($data) {
				
	$HuisNr=$data["Huisnr"].$data["HuisLetter"];
	
	$sql  = 'INSERT INTO oostpoort_adres ( ';
	$sql .= '`Adr_Straat`, ';
	$sql .= '`Adr_Huisnr`, ';
	$sql .= '`Adr_Plaats`, ';
	$sql .= '`Adr_Blk_Id`, ';
	$sql .= '`Adr_Postcode` ';
	$sql .= ') VALUES (';
	$sql .= '"'.$data["Straatnaam"].'", ';
	$sql .= '"'.$HuisNr.'", ';
	$sql .= '"'.$data["Woonplaats"].'", ';
	$sql .= oostpoort_convert_block($data).', ';
	$sql .= '"'.$data["Postcode"].'") ';
		
	oostpoort_debug_sql($sql);
	db_query($sql); 	
	
	return oostpoort_find_address($data);
}

function oostpoort_update_address($data, $AdrId) {

	$sql  = 'UPDATE ';
	$sql .= '  oostpoort_adres ';
	$sql .= 'SET ';
	$sql .= '  Adr_Blk_Id='.oostpoort_convert_block($data).' ';
	$sql .= 'WHERE ';
	$sql .= '  Adr_Id='.$AdrId;
	
	oostpoort_debug_sql($sql);
	db_query($sql); 	
}
	
function oostpoort_insert_member($data, $AdrId)  {
	
	$name = oostpoort_split_name($data["Aanschrijfnaam"]);
	
	$sql  = 'INSERT INTO oostpoort_persoon ( ';
	$sql .= '`Prs_Kerklidnummer`, ';
	$sql .= '`Prs_KringlidJN`, ';
	$sql .= '`Prs_Voorletters`, ';
	$sql .= '`Prs_Tussenvoegsels`, ';
	$sql .= '`Prs_Voornamen`, ';
	$sql .= '`Prs_Achternaam`, ';
	$sql .= '`Prs_GeslachtMV`, ';
	$sql .= '`Prs_Gem_ID`, ';
	$sql .= '`Prs_Bst_ID`, ';
	$sql .= '`Prs_Huidig_Adr_ID`, ';
	$sql .= '`Prs_Geboortedatum`, ';
	$sql .= '`Prs_GeboortePlaats`, ';
	$sql .= '`Prs_Huwelijksdatum`, ';
	$sql .= '`Prs_DoopDatum`, ';
	$sql .= '`Prs_BelijdenisDatum`, ';
	$sql .= '`Prs_ImportNieuwdatum`, ';
	$sql .= '`Prs_ImportWijzigingdatum` ';
	$sql .= ') VALUES (';
	$sql .= '"'.$data["KerkLidNr"].'", ';
	$sql .= '"ONWAAR", ';
	$sql .= '"'.$name['Initials'].'", ';
	$sql .= '"'.$name['MiddleName'].'", ';
	$sql .= '"'.$data["Voornamen"].'", ';		
	$sql .= '"'.$name['LastName'].'", ';
	if ($data["Geslacht"]=='Man') { 
		$sql .= '"M", ';
	} else { 
		$sql .= '"V", '; 
	}
	$sql .= '0, ';
	$sql .= oostpoort_member_state($data["Burgelijkstaat"]).', ';
	$sql .= $AdrId.', ';
	$sql .= '"'.oostpoort_convert_date_reverse($data["GeboorteDatum"]).'", ';
	$sql .= '"'.$data["GeboortePlaats"].'", ';
	$sql .= '"'.oostpoort_convert_date_reverse($data["HuwelijkDatum"]).'", ';
	$sql .= '"'.oostpoort_convert_date_reverse($data["DoopDatum"]).'", ';
	$sql .= '"'.oostpoort_convert_date_reverse($data["BelijdenisDatum"]).'", ';
	$sql .= '"'.date("Y-m-d").'", ';
	$sql .= '"'.date("Y-m-d").'")';
	
	oostpoort_debug_sql($sql);
	db_query($sql); 
}

function oostpoort_update_member($data,$PrsId,$AdrId) {

	$name = oostpoort_split_name($data["Aanschrijfnaam"]);
	
	$sql  = 'UPDATE ';
	$sql .= '  oostpoort_persoon ';
	$sql .= 'SET ';
	$sql .= '  Prs_Voornamen="'.$data["Voornamen"].'", ';
	$sql .= '  Prs_Voorletters="'.$name['Initials'].'", ';
	$sql .= '  Prs_Tussenvoegsels="'.$name['MiddleName'].'", ';
	$sql .= '  Prs_Voornamen="'.$data["Voornamen"].'", ';
	$sql .= '  Prs_Achternaam="'.$name['LastName'].'", ';
	$sql .= '  Prs_Bst_ID='.oostpoort_member_state($data["Burgelijkstaat"]).', ';
	$sql .= '  Prs_Geboortedatum="'.oostpoort_convert_date_reverse($data["GeboorteDatum"]).'", ';
	$sql .= '  Prs_GeboortePlaats="'.$data["GeboortePlaats"].'", ';
	$sql .= '  Prs_Huwelijksdatum="'.oostpoort_convert_date_reverse($data["HuwelijkDatum"]).'", ';
	$sql .= '  Prs_DoopDatum="'.oostpoort_convert_date_reverse($data["DoopDatum"]).'", ';
	$sql .= '  Prs_BelijdenisDatum="'.oostpoort_convert_date_reverse($data["BelijdenisDatum"]).'", ';
	$sql .= '  Prs_Huidig_Adr_ID='.$AdrId.', ';
	$sql .= '  Prs_ImportWijzigingdatum="'.date("Y-m-d").'" ';
	$sql .= 'where ';
	$sql .= '  Prs_id='.$PrsId;
		
	oostpoort_debug_sql($sql);
	db_query($sql); 	
}
	
function oostpoort_split_name($origName)
{
	// Get Initials
	list( $Voorletters ) = split(' ', $origName);
	$name['Initials']=$Voorletters;
	
	// Get Middle name
	$offset=1;
	$tmp=substr($origName, strlen($Voorletters)+$offset);
	
		
	if (preg_match("/^[vV]an der /", $tmp)) {
		$VoorvoegselNaam="van der";
	} else if (preg_match("/^[vV]an den /", $tmp)) {
		$VoorvoegselNaam="van den";
	} else if (preg_match("/^[vV]an de /", $tmp)) {
		$VoorvoegselNaam="van de";
	} else if (preg_match("/^[vV]an 't /", $tmp)) {
		$VoorvoegselNaam="van 't";	
	} else if (preg_match("/^[vV]an /", $tmp)) {
		$VoorvoegselNaam="van";	
	} else if (preg_match("/^[tT]en /", $tmp)) {
		$VoorvoegselNaam="ten";						
	} else if (preg_match("/^[iI]n 't /", $tmp)) {
		$VoorvoegselNaam="in 't";
	} else if (preg_match("/^[dD]en /", $tmp)) {
		$VoorvoegselNaam="den";
	} else if (preg_match("/^[dD]e /", $tmp)) {
		$VoorvoegselNaam="de";
	}
	$name['MiddleName']=$VoorvoegselNaam;
	if ($name['MiddleName']!="") $offset+=1;		
	
	// Get Last name
	$Achternaam=substr($origName, strlen($Voorletters)+strlen($VoorvoegselNaam)+$offset);
	$name['LastName']=$Achternaam;
	
	return $name;
}

function oostpoort_find_member($data) {

	$name = oostpoort_split_name($data["Aanschrijfnaam"]);

	// First try to match member with Church Member Id
	if ($KerkLidNr!=0) {
	
		$sql  = 'select ';
		$sql .= '  Prs_ID as PrsId ';
		$sql .= 'from ';
		$sql .= '  oostpoort_persoon ';
		$sql .= 'where ';
		$sql .= '  Prs_Kerklidnummer="'.$data["KerkLidNr"].'"';
		
		oostpoort_debug_sql($sql);
		$queryResult = db_query($sql);     
		$result = db_fetch_object($queryResult);
	
		$PrsId = $result->PrsId;
	}
	
	// Second try to match the member by name
	if ($PrsId=="") {
		$sql  = 'select ';
		$sql .= '  Prs_ID as PrsId ';
		$sql .= 'from ';
		$sql .= '  oostpoort_persoon ';
		$sql .= 'where ';
		$sql .= '  Prs_Voorletters="'.$name['Initials'].'" and ';
		$sql .= '  Prs_Tussenvoegsels="'.$name['MiddleName'].'" and ';
		$sql .= '  Prs_Achternaam="'.$name['LastName'].'"';

		oostpoort_debug_sql($sql);
		$queryResult = db_query($sql);     
		$result = db_fetch_object($queryResult);
		
		$PrsId = $result->PrsId;
	}
	
	oostpoort_debug_param('PrsId',$PrsId);	
	return $PrsId;
}

function oostpoort_member_state($TypeOrig) {
	
	$Type = ucfirst($TypeOrig);
	
	$sql  = 'select ';
	$sql .= ' Bst_ID as BstId ';
	$sql .= 'from  ';
	$sql .= '  oostpoort_burgelijkestaat ';
	$sql .= 'where ';
	$sql .= ' Bst_Code like "'.$Type.'"';
  
	oostpoort_debug_sql($sql);
	$queryResult = db_query($sql);     
	$result = db_fetch_object($queryResult);
		
	$BstId = $result->BstId;
	oostpoort_debug_param('BstId',$BstId);
	return $BstId;
}

function oostpoort_convert_block($data) {
	
	$block=5;
	
	list( $Nummer, $Letters ) = split(' ', $data["Postcode"]);
	
	try
  	{
		if ($Nummer=="2806") {		
			$block=1;
		} else if (($Nummer=="2807") 
					&& (preg_match("/[Mm]olen/", $data['Straatnaam']) 
					|| preg_match("/[D-H][A-B]/", $Letters))) { 
			$block=2;
		} else if (($Nummer=="2807") 
					&& ( preg_match("/[Pp]older/", $data['Straatnaam']) 
					 	|| preg_match("/[Vv]rijheids/", $data['Straatnaam']) 
					 	|| preg_match("/[K-N][A-Z]/", $Letters) 
					 	|| preg_match("/[R-R][A-Z]/", $Letters)
				   	)) {
			$block=4;
		} else if ($Nummer=="2807") {
			$block=3;
		} else {
			$block=5;
		}
  	} catch(Exception $e) {
  		//echo 'Line: ['. $Nummer.'] [' .$Letters.'] ['.$data['Straatnaam'].']';
  		//echo 'Message: ' .$e->getMessage();
  	}  		
	return $block;
}

// ##################################################################################
// THE END
// ##################################################################################