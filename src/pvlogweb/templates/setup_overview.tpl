<h2>Logical Plants:</h2>
<table>
<tbody>
	<tr>
		{{#LOGICAL_PLANTS}}
		<td>
			<h4>
				<a href="setup?view=logical_plant&logical_plant={{LOGICAL_PLANT_ID}}">{{LOGICAL_PLANT_NAME}}</a>
			</h4>
			<ul>
				{{#INVERTERS}}
				<li>
					<a href="setup?view=inverter&inverter={{INVERTER_ID}}">{{INVERTER_NAME}} ({{PLANT_NAME}}</a>
				</li>
				{{/INVERTERS}}
			<ul>
		</td>
		{{/LOGICAL_PLANTS}}
	</tr>
</tbody>
</table>
<h2>Wired Plants:</h2>
<ul>
	{{#PLANTS}}
	<li>
		<a href="setup?view=plant&plant={{PLANT_ID}}">{{PLANT_NAME}}</a>
	</li>
	{{/PLANTS}}
</ul>
