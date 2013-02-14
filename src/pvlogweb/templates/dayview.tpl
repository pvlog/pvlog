<div id="content">
	<div id="chart">
	</div>
	
	<script type="text/javascript">
	$(function () {
		var options = {
			xaxis: {
				mode: "time",
				timeformat: "%H:%M"
			}
		};
		
	
		var values = 	
		{{#CHART_DATA_SERIES}}
		[{label: {{CHART_LABEL}, data: 
			[
			{{#CHART_DATA_VALUES}}	
				[{{CHART_DATA_X}}, {{/CHART_DATA_Y}}] {{/CHART_DATA_VALUES_seperator}}, {{#CHART_DATA_VALUES_seperator}}
			{{/CHART_DATA_VALUES}}
		{{/CHART_DATA_SERIES}}
			];
	
		$.plot($("#chart"), values, options);
	});
	</script>
	
	<div id="selection">
		<form>
			<div class="select_multiple">
				<label for="inverter">{{CHOOSE_INVERTER}}</label>
				<select name="inverter" multiple="">
					{{#INVERTERS}}
					<option>{{INVERTER_NAME}}</option>
					{{/INVERTERS}}
				</select>
			</div>
			<div class="select">
				<label for="year">{{CHOOSE_YEAR}}</label>
				<select name="year">
					{{#YEARS}}
					<option>{{YEAR}}</option>
					{{/YAERS}}
				</select>
			</div>
			<div class="select">
				<label for="month">{{CHOOSE_MONTH}}</label>
				<select name="month">
					{{#MONTHS}}
					<option>{{MONTH}}</option>
					{{/MONTHS}}
				</select>
			</div>
			<div class="select">
				<label for="day">{{CHOOSE_DAY}}</label>
				<select name="day">
					{{#DAYS}}
					<option>{{DAY}}</option>
					{{/DAYS}}
				</select>
			</div>
			<div class="radio">
				<label for="type">{{CHOOSE_TYPE}}</label>
				{{AC}}<input type="radio" name="type" value="ac"/> 
				{{DC}}<input type="radio" name="type" value="dc"/>
			</div>
			<ul>
				{{#INVERTERS}}
				<li>
					{{INVERTER}}
					{{#LINES}}
					<div class="radio">
						<label for="line">{{LINE}}</label>
						<input type="radio" name="{{INVERTER}}" value="{{LINE}}"/>
					</div>
					{{/LINE}}
				</li>
				{{/INVERTERS}}
			</ul>
		</form>
	</div> <!-- selection -->
</div> <!-- content -->

<script type="text/javascript" src="script/jqery.js"></script>
<script type="text/javascript" src="script/jqery.flot.js"></script>
<!--[if lte IE 8]><script language="javascript" type="text/javascript" src="script/excanvas.min.js"></script><![endif]-->
