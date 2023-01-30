tool
extends TileMap

var tiles = {}
var buffer_size = 32

var advanced_mode = false
var safety_margin = 2
var allocation_cells_visited = 10000
var allocation_cells_calculation = 1000
var allocation_target_cells = 100

onready var flow_field_module = preload("res://addons/FlowField/FlowFieldModule.gdns").new()

func _ready():
	_set_tile_map_tiles_cost()
	set_buffer_size(buffer_size)
	_load_play_field()

func reset_cost_map():
	flow_field_module.reset_cost_map()

func set_buffer_size(buffer_size:int) -> void:
	flow_field_module.set_buffer_size(buffer_size)

func set_cell(x: int, y: int, tile:int, flip_x: bool = false, flip_y: bool = false, 
				transpose: bool = false, autotile_coord: Vector2 = Vector2( 0, 0 )):
	.set_cell(x,y,tile)
	update_cell_cost(x,y,tiles[tile]["cost"])

func reset_cell(x:int, y:int):
	var tile = get_cell(x,y)
	update_cell_cost(x,y,tiles[tile]["cost"])

func _load_play_field() -> void:
	flow_field_module.set_play_field(self, tiles,
	 allocation_cells_visited, allocation_target_cells, allocation_cells_calculation)

func create_flow_field(target_position:Vector2, important_positions:Array=[]) -> void:
	var important_cells = []
	for position in important_positions:
		if not typeof(position) == TYPE_VECTOR2:
			return
		important_cells.append(world_to_map(position))
	var target_cell = world_to_map(target_position)
	
	flow_field_module.create_flow_field([target_cell],important_cells, safety_margin)

func create_flow_fieldv(target_positions:Array, important_positions:Array=[]) -> void:
	var target_cells = []
	for position in target_positions:
		if not typeof(position) == TYPE_VECTOR2:
			return
		target_cells.append(world_to_map(position))
	
	var important_cells = []
	for position in important_positions:
		if not typeof(position) == TYPE_VECTOR2:
			return
		important_cells.append(world_to_map(position))

	flow_field_module.create_flow_field(target_cells,important_cells, safety_margin)

func create_flow_field_cell(target_cell:Vector2, important_cells:Array=[]) -> void:
	for cell in important_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return
	flow_field_module.create_flow_field([target_cell],important_cells, safety_margin)

func create_flow_field_cellv(target_cells:Array, important_cells:Array=[]) -> void:
	for cell in target_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return
	for cell in important_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return

	flow_field_module.create_flow_field(target_cells,important_cells, safety_margin)


func update_cell_cost(cell_x:int, cell_y:int, cell_cost:int) -> void:
	flow_field_module.update_cell_cost(cell_x, cell_y, cell_cost)

func reduce_map_size():
	flow_field_module.reduce_map_size()

func get_direction(position:Vector2, target_position:Vector2=Vector2(65535, 65535)) -> Vector2:
	if (target_position.x == 65535 and target_position.y == 65535):
		return flow_field_module.get_direction(position, false, Vector2.ZERO)
	return flow_field_module.get_direction(position, true, target_position)

func get_direction_cell(cell_x:int, cell_y:int) -> Vector2:
	return flow_field_module.get_direction(map_to_world(Vector2(cell_x, cell_y)), false, Vector2.ZERO)


func get_distance(cell:Vector2) -> int:
	return flow_field_module.get_cell_distance(cell)

func _get_property_list():
	var array_to_return = []
	_set_tile_map_tiles_cost()
	array_to_return.append({
		"name": "advanced_mode_activated",
		"type": 1,
	})
	

	var tile_costs = []

	for tile in tiles:
		tile_costs.append({
			"name": "tile_costs/" + str(tiles[tile].name),
			"type": 2,
		})
	for item in tile_costs:
		array_to_return.append(item)
	
	if advanced_mode:
		array_to_return.append(
			{"name": "advanced_mode/safety_margin",
			"type" : 2,})
		array_to_return.append(
			{"name": "advanced_mode/allocation_cells_visited",
			"type" : 2,})
		array_to_return.append(
			{"name": "advanced_mode/allocation_cells_calculation",
			"type" : 2,})
		array_to_return.append(
			{"name": "advanced_mode/allocation_target_cells",
			"type" : 2,})
		
	property_list_changed_notify()
	return array_to_return

func _set(property, value):
	_set_tile_map_tiles_cost()
	if "tile_costs" in property:
		if not is_instance_valid(tile_set):
			return;
		var name = property.substr(len("tile_costs") + 1)
		var nr = tile_set.find_tile_by_name(name)
		if value < 0:
			value = 0
		tiles[nr].cost = value
	elif property == "advanced_mode/safety_margin":
		if value < 0:
			safety_margin = 0
		else:
			safety_margin = value
	elif property == "advanced_mode/allocation_cells_calculation":
		if value <= 0:
			allocation_cells_calculation = 1
		else:
			allocation_cells_calculation = value
	elif property == "advanced_mode/allocation_cells_visited":
		if value <= 0:
			allocation_cells_visited = 1
		else:
			allocation_cells_visited = value
	elif property == "advanced_mode/allocation_target_cells":
		if value <= 0:
			allocation_target_cells = 1
		else:
			allocation_target_cells = value

	elif property == "advanced_mode_activated":
		advanced_mode = value
		property_list_changed_notify()

func _get(property):
	if "tile_costs" in property:
		if not is_instance_valid(tile_set):
			return;
		var name = property.substr(len("tile_costs") + 1)
		var nr = tile_set.find_tile_by_name(name)
		return tiles[nr].cost
	elif property == "advanced_mode_activated":
		return advanced_mode
	elif property == "advanced_mode/safety_margin":
		return safety_margin
	elif property == "advanced_mode/allocation_cells_calculation":
		return allocation_cells_calculation
	elif property == "advanced_mode/allocation_cells_visited":
		return allocation_cells_visited
	elif property == "advanced_mode/allocation_target_cells":
		return allocation_target_cells
	

func _set_tile_map_tiles_cost():
	if not is_instance_valid(tile_set):
		return;
	
	var updated_tiles = {}
	
	for id in tile_set.get_tiles_ids():
		var name = tile_set.tile_get_name(id)
		if not tiles.has(id):
			tiles[id] = {"name": name,
						"cost": 1}
		else:
			tiles[id].name = name
		updated_tiles[id] = true
	
	var ids_to_delete = {}
	for id in tiles:
		if not updated_tiles.has(id):
			ids_to_delete[id] = true
	
	for id in ids_to_delete:
		tiles.erase(id)

