extends Node2D

var random_field = []
var all_arrows = {}

onready var flow_field = $FlowField
onready var label_time_update = $"CanvasLayer/Control/VBoxContainer/update Play Field/Time"
onready var label_time_create = $"CanvasLayer/Control/VBoxContainer/create Flow Field/Time"
onready var label_nr_of_cells = $"CanvasLayer/Control/VBoxContainer/Number of Cells/Time"
onready var label_access_time = $"CanvasLayer/Control/VBoxContainer/Access/Time"

onready var enemy_scene = preload("res://Enemy.tscn")
onready var arrow_scene = preload("res://Arrow.tscn")
var target_position = Vector2(0,0)

var special_cost_map = {}
var enemies = []
var enemies_position = []
var max_enemies = 0

func _ready():
	SignalBus.connect("enemy_disappeared", self, "delete_enemy")
	create_arrows()
	#flow_field.load_play_field()
	print("!!!!")
	
	
func update_flow_field(cell):
	print("update begin")
	var begin_time = OS.get_ticks_usec()
	
	#flow_field.update_play_field()
	var end_time = OS.get_ticks_usec()
	
	label_time_update.text = str((end_time - begin_time) / 1000.0)
	
	enemies_position = []
	for enemy in enemies:
		enemies_position.append(flow_field.world_to_map(enemy.position))
	
	begin_time = OS.get_ticks_usec()
	flow_field.create_flow_field_cell(cell, enemies_position)
	end_time = OS.get_ticks_usec()
	
	label_time_create.text = str((end_time - begin_time) / 1000.0)
	label_nr_of_cells.text = str(flow_field.get_used_cells().size())
	print("update end")
	
	update_arrows()

func delete_enemy(enemy):
	enemies.erase(enemy)

func create_arrows():
	flow_field.get_used_cells()
	for cell in flow_field.get_used_cells():
		var arrow = arrow_scene.instance()
		arrow.position = (cell + Vector2(0.5,0.5)) * flow_field.cell_size
		add_child(arrow)
		all_arrows[cell] = arrow

func update_arrows():
	var flow_field_result = []
	for cell in all_arrows:
		all_arrows[cell].rotation = 0
		all_arrows[cell].rotate(flow_field.get_direction((cell + Vector2(0.5, 0.5)) * flow_field.cell_size).angle() + deg2rad(90) )
		flow_field_result.append(flow_field.get_direction((cell + Vector2(0.5, 0.5))* flow_field.cell_size))
	#print(flow_field_result)

func measure_access_speed():
	var i = 0
	var begin_time = OS.get_ticks_usec()
	while (i < 10000):
		i+= 1
		var direction = flow_field.get_direction(Vector2(0,0))
	
	var end_time = OS.get_ticks_usec()
	label_access_time.text = str((end_time - begin_time) / 1000.0)
	
	begin_time = OS.get_ticks_usec()
	for j in range(10000):
		empty()
	end_time = OS.get_ticks_usec()
	print("Access time empty: " + str((end_time - begin_time) / 1000.0))

func empty():
	pass

func spawn_enemies(amount=1000):
	var spawned = 0
	var count = 0
	for cell in all_arrows:
		count += 1
		if count < 1000:
			continue
		if len(enemies) > max_enemies:
			break
		#if (randf() > 0.5):
		#	continue
		var new_enemy = enemy_scene.instance()
		new_enemy.set_flow_field(flow_field)
		new_enemy.set_target_position(get_global_mouse_position())
		new_enemy.position = cell * flow_field.cell_size
		add_child(new_enemy)
		enemies.append(new_enemy)
		spawned += 1
	
	print("Spawned: " + str(spawned) + " enemies")

func _process(delta):
	#print("Enemies left: " + str(len(enemies)))
	var mous_position = get_global_mouse_position()
	var cell:Vector2 = mous_position / flow_field.cell_size
	#enemies_position = []
	#for enemy in enemies:
#		enemies_position.append(flow_field.world_to_map(enemy.position))
	
	#var start_time2 = OS.get_ticks_usec()
	#flow_field.create_flow_field(cell, enemies_position)
	#var end_time2 = OS.get_ticks_usec()
	#print((end_time2 - start_time2) / 1000.0)
	
	if Input.is_action_just_pressed("ui_accept"):
		print("pressed")
		spawn_enemies()
		mous_position = get_global_mouse_position()
		cell = mous_position / flow_field.cell_size
		cell = cell.floor()
		update_flow_field(cell)
		measure_access_speed()
		
	
	if Input.is_action_just_pressed("select_cell"):
		#var start_time = OS.get_ticks_usec()
		mous_position = get_global_mouse_position()
		cell = flow_field.world_to_map(mous_position)
		#flow_field.reset_cell(cell.x, cell.y)
		#var cell:Vector2 = mous_position / flow_field.cell_size
		
		#print(str(flow_field.get_direction(mous_position, target_position)))
		#print(str(flow_field.get_direction(cell)))
		#print(str(mous_position))
		#print(str(cell))
		
		special_cost_map = {}
		special_cost_map[cell] = 30
		
		#print(special_cost_map)
		
		var start_time = OS.get_ticks_usec()
		flow_field.update_cell_cost(cell.x, cell.y, 30)
		
		var end_time = OS.get_ticks_usec()
		
		print("time needed: " , str((end_time - start_time) / 1000.0))
		
	if Input.is_action_just_pressed("change_target_position"):
		#print("new target position: " + str(target_position))
		target_position = get_global_mouse_position()
		#flow_field.update_play_field()
	
	if Input.is_action_just_pressed("reset_cost_map"):
		var begin_time = OS.get_ticks_usec()
		flow_field.reset_cost_map()
		var end_time = OS.get_ticks_usec()
		print("Reset time: " + str((end_time - begin_time) / 1000.0))
	
	if Input.is_action_just_pressed("reduce_map_size"):
		var begin_time = OS.get_ticks_usec()
		flow_field.reduce_map_size()
		var end_time = OS.get_ticks_usec()
		print("Reset time: " + str((end_time - begin_time) / 1000.0))
	if Input.is_action_just_pressed("reset_cell"):
		mous_position = get_global_mouse_position()
		cell = flow_field.world_to_map(mous_position)
		flow_field.reset_cell(cell.x, cell.y)
