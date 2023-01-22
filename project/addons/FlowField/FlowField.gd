tool
extends TileMap

var tiles = {}
onready var flow_field_module = preload("res://addons/FlowField/FlowFieldModule.gdns").new()

func _ready():
	_set_tile_map_tiles_cost()
	update_play_field()

func reset_cost_map():
	flow_field_module.reset_cost_map()

func set_buffer_size(buffer_size:int) -> void:
	flow_field_module.set_buffer_size(buffer_size)

func set_cell(x: int, y: int, tile:int, flip_x: bool = false, flip_y: bool = false, transpose: bool = false, autotile_coord: Vector2 = Vector2( 0, 0 )):
	.set_cell(x,y,tile)
	update_cell_cost(x,y,tiles[tile])

func update_play_field() -> void:
	flow_field_module.set_play_field(self, tiles)

func create_flow_field(target_cell:Vector2, important_cells:Array=[]) -> void:
	for cell in important_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return
	flow_field_module.create_flow_field([target_cell],important_cells)

func create_flow_fieldv(target_cells:Array, important_cells:Array=[]) -> void:
	for cell in target_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return
	for cell in important_cells:
		if not typeof(cell) == TYPE_VECTOR2:
			return

	flow_field_module.create_flow_field(target_cells,important_cells)


func update_cell_cost(cell_x:int, cell_y:int, cell_cost:int) -> void:
	flow_field_module.update_cell_cost(cell_x, cell_y, cell_cost)

func reduce_map_size():
	flow_field_module.reduce_map_size()

func get_direction(position:Vector2, target_position:Vector2=Vector2(65535, 65535)) -> Vector2:
	if (target_position.x == 65535 and target_position.y == 65535):
		return flow_field_module.get_position_direction(position, false, Vector2(0,0))
	return flow_field_module.get_position_direction(position, true, target_position)

func get_cell_distance(cell:Vector2) -> int:
	return flow_field_module.get_cell_distance(cell)

func _get_property_list():
	var array_to_return = []
	_set_tile_map_tiles_cost()
	property_list_changed_notify()
	
	if true:
		var extra_properties = []

		for tile in tiles:
			extra_properties.append({
				"name": "tile_costs/" + str(tiles[tile].name),
				"type": 2,
			})
		for item in extra_properties:
			array_to_return.append(item)
	
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

func _get(property):
	if "tile_costs" in property:
		if not is_instance_valid(tile_set):
			return;
		var name = property.substr(len("tile_costs") + 1)
		var nr = tile_set.find_tile_by_name(name)
		return tiles[nr].cost

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

