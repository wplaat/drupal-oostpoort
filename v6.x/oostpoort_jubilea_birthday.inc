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
// State Machine
// ##################################################################################

function oostpoort_jubilea_birthday() {
	
	//echo var_dump($_POST);
	
	// *****************************
	// Check access for this page.
	// *****************************
	
	if (!oostpoort_visit_member_access()) {
		return drupal_access_denied();
	}
	
	// *****************************
	// Process POST variables
	// *****************************
	
	$Action=$_POST['Action'];
	oostpoort_debug_param("Action", $Action);
	       
	// *****************************
	// Process statemachine
	// *****************************
 	
	switch ($Action) {
   
		case 'MemberView':
			// Create Page
			$page = oostpoort_member_view(true);     	
     		$page .= '<a href="'.url(OOSTPOORT_JUBILEA_BIRTHDAY).'">'.t('Terug').'</a>';     	

     		// Overwrite default breadcrumb menu
			$node["child1"] = '<a href="'.url(OOSTPOORT_JUBILEA_BIRTHDAY).'">'.t('Verjaardagsjubilaris Rapport').'</a>';
			$node["child2"] = t('Lid Detail');
			oostpoort_breadcrumb($node);
			break;
		
		default: 
			$page = oostpoort_jubilea_birthday_list();
			break;
	}
	
	print theme("page", $page);
}

// ##################################################################################
// Views
// ##################################################################################

function oostpoort_jubilea_birthday_list() {

	// *****************************
	// Create page
	// *****************************

	// build page
	$page .= '<form name="OostpoortForm" method="POST" >';
	$page .= '<div class="oostpoort">';
		
	// Breadcrumb menu
	$node["child1"] = '<a href="'.url(OOSTPOORT_JUBILEA_BIRTHDAY).'">'.t('Verjaardagsjubilaris Rapport').'</a>';
	oostpoort_breadcrumb($node);

	// title
	drupal_set_title(t('Verjaardagsjubilaris Rapport'));
		
	$page .= '<fieldset>' ;
	$page .= '<b>'.t('Rapport loopt over de volgende periode:').'</b>'.'<br/>';
	$page .= t('2 maanden terug t/m 4 maanden vooruit.');
	$page .= '</fieldset>' ;

	$x=array(50, 65, 70, 75, 80, 85, 90, 95, 100);
		
	foreach ($x as $AgeId)
	{
		$page .= '<fieldset>' ;
		$page .= '<legend><i>';
		$page .= t('Verjaardagsjubilarrisen').' ';
		if ($AgeId<100) {
			$page .= $AgeId.' '.t('jaar');
		} else {
			$page .= $AgeId.' '.t('jaar en ouder');
		}
		$page .= '</i></legend>';
	
		$periode_start=oostpoort_offset_date( -2*31 );
		list($year, $month, $day) = split('-', $periode_start);
		$yearStart=$year-$AgeId;
		$startDate = $yearStart."-".$month."-".$day;
	
		$periode_end=oostpoort_offset_date( 4*31 );
		list($year, $month, $day) = split('-', $periode_end);
		$yearEnd=$year-$AgeId;
		$endDate = $yearEnd."-".$month."-".$day;
			
		$query = 'select ';
		$query .= ' a.Prs_Id as PrsId, ';
		$query .= ' concat(IF(a.Prs_Roepnaam="",a.Prs_Voorletters, a.Prs_Roepnaam)," ", a.Prs_Tussenvoegsels, " ", a.Prs_Achternaam) as Naam, ';
		$query .= ' a.Prs_GeslachtMV as PrsGeslacht, ';
		$query .= ' a.Prs_EmailAdres as PrsEmail, ';
		$query .= ' a.Prs_TelefoonMobiel as PrsMobiel, ';
		$query .= ' a.Prs_Geboortedatum as PrsGeboortedatum, ';
		$query .= ' a.Prs_Huwelijksdatum as PrsHuwelijksdatum, ';
		$query .= ' concat(b.Adr_Straat, " " ,b.Adr_Huisnr) as Straat, ';
		$query .= ' b.Adr_Plaats as AdrPlaats, ';
		$query .= ' b.Adr_Postcode as AdrPostcode, ';	
		$query .= ' b.Adr_Telefoon as AdrTelefoon ';	
		$query .= 'from ';
		$query .= '  oostpoort_persoon a, oostpoort_adres b ';
		$query .= 'where ';
		$query .= ' a.Prs_Huidig_Adr_ID=b.Adr_id and ';
		if ($AgeId!=100) {
			$query .= ' a.Prs_Geboortedatum >= "'.$startDate.'" and ';
			$query .= ' a.Prs_Geboortedatum <= "'.$endDate.'" and ';
		} else {
			$query .= ' a.Prs_Geboortedatum <= "'.$startDate.'" and ';
		}
		$query .= ' a.Prs_Kerklidnummer != 0 ';
		$query .= 'order by ';
		$query .= ' a.Prs_Geboortedatum';

		oostpoort_debug_sql($query);
		$queryResult = db_query($query);
		$page .= oostpoort_jubilea_birthday_table($queryResult,1);
		
		$page .= '</fieldset>';

	}
	$page .= '<a href="'.url(OOSTPOORT_MENU).'">'.t('Terug').'</a>';
	
	$page .= '</div>';		
	$page .= '</form>';
	
	return $page;
}

function oostpoort_jubilea_birthday_table($queryResult, $mode) {
		
	$page .= '<table>';

   $page .= '<thead>';
	// Show Banner
	$page .= '<tr>';
	$page .= '<th><b>'.t('Naam').'</b></th>';
	$page .= '<th><b>'.t('Straat').'</b></th>';
	$page .= '<th><b>'.t('Plaats').'</b></th>';
	$page .= '<th><b>'.t('Postcode').'</b></th>';
	$page .= '<th class="active"><b>'.t('Geboren').'</b></th>';
    //$page .= '<th><b>'.t('Huwelijk datum').'</b></th>';
	//$page .= '<th><b>'.t('Telefoon').'</b></th>';
	$page .= '</tr>';
	$page .= '</thead>';
   $page .= '<tbody>';
	// Show all found members
	$page_tmp='';
	while ($data = db_fetch_object($queryResult))
	{
		if ((++$i%2)==0) {
			$page_tmp .= '<tr class="even">';
		} else {
			$page_tmp .= '<tr class="odd">';
		}
				
     	$page_tmp .= '<td>';
     	$page_tmp .= oostpoort_hiddenlink('OostpoortForm', 'PrsId', $data->PrsId, 'MemberView', $data->Naam);
     	$page_tmp .= '</td>';
		
		$page_tmp .= '<td>'.$data->Straat.'</td>';
		$page_tmp .= '<td>'.$data->AdrPlaats.'</td>';  
		$page_tmp .= '<td>'.$data->AdrPostcode.'</td>';
		$page_tmp .= '<td class="active">'.oostpoort_convert_date($data->PrsGeboortedatum).'</td>';
		//$page_tmp .= '<td>'.oostpoort_convert_date($data->PrsHuwelijksdatum).'</td>';
		//$page_tmp .= '<td>'.$data->AdrTelefoon.'</td>';
		$page_tmp .= '</tr>';
	}

	if ( $page_tmp!='') {
	
		$page .= $page_tmp;
		
	} else  {
		$page .= '<tr>';
		$page .= '<td>'.t('Geen informatie gevonden').'</td>';
		$page .= '</tr>';
	}
	
	$page .= '</tbody>';
	$page .= '</table>';

	return $page;
}

// ##################################################################################
// THE END
// ##################################################################################