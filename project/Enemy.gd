extends Sprite


var flow_field
var target_position: Vector2
var speed = 300

func _process(delta):
	var direction:Vector2 = flow_field.get_exact_position_direction(self.position, target_position)
	if (self.position.distance_to(target_position)  < delta * speed * 2):
		self.queue_free()
		return
	direction = direction.normalized()
	self.position += direction * delta * speed

func set_flow_field(new_flow_field):
	flow_field = new_flow_field

func set_target_position(new_target_position):
	target_position = new_target_position
