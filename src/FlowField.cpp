#include "FlowField.hpp"
#include <math.h> 




void FlowField::_register_methods() {
	godot::register_method("get_direction", &FlowField::get_direction);
	godot::register_method("set_play_field", &FlowField::set_play_field);
	godot::register_method("get_cell_distance", &FlowField::get_cell_distance);
	godot::register_method("update_cell_cost", &FlowField::update_cell_cost);
	godot::register_method("create_flow_field", &FlowField::create_flow_field);
	godot::register_method("set_buffer_size", &FlowField::set_buffer_size);
	godot::register_method("reset_cost_map", &FlowField::reset_cost_map);
	godot::register_method("reduce_map_size", &FlowField::reduce_map_size);

	
}


godot::Vector2 FlowField::get_direction(godot::Vector2 position, bool consider_target_position, godot::Vector2 target_position){
	godot::Vector2 direction;
	int cell[2];
	double offset_position[2];
	cell[0] = floor(position.x / cell_size[0]) + offset_x;
	cell[1] = floor(position.y / cell_size[1]) + offset_y;

	if (!is_in_play_field(cell))
	{
		return direction;
	}
	//direction.x = direction_distance[cell[0]][cell[1]][0];
	//direction.y = direction_distance[cell[0]][cell[1]][1];
	//return direction;

	offset_position[0] = ((fmod(position.x, cell_size[0]) / cell_size[0])  - 0.5 * ( (0 <= position.x) - (position.x < 0))) * cost_map[cell[0]][cell[1]];
	offset_position[1] = ((fmod(position.y, cell_size[1]) / cell_size[1])  - 0.5 * ( (0 <= position.y) - (position.y < 0))) * cost_map[cell[0]][cell[1]];


	direction.x = direction_distance[cell[0]][cell[1]][0] - offset_position[0];
	direction.y = direction_distance[cell[0]][cell[1]][1] - offset_position[1];


	if (!consider_target_position){
		return direction;
	}

	if (!target_visible[cell[0]][cell[1]]){
		return direction;
	}

	int target_cell[2];
	target_cell[0] = floor (target_position.x / cell_size[0]) + offset_x;
	target_cell[1] = floor (target_position.y / cell_size[1]) + offset_y;

	// Not in playfield
	if (!is_in_play_field(target_cell)){
		return direction;
	}

	// cell does not track anything
	int tracked_index = target_cells_saved[cell[0]][cell[1]];

	if (tracked_index == -1){
		return direction;
	}

	int tracked_cell[2];
	tracked_cell[0] = index_to_target_cell[tracked_index][0];
	tracked_cell[1] = index_to_target_cell[tracked_index][1];

	// not the tracked target by the cell
	if (tracked_cell[0] != target_cell[0] || tracked_cell[1] != target_cell[1]){
		return direction;
	}

	double target_position_offset[2];
	target_position_offset[0] = ((fmod (target_position.x, cell_size[0]) / cell_size[0]) - 0.5 * ( (0 <= position.x) - (position.x < 0))) * cost_map[cell[0]][cell[1]];
	target_position_offset[1] = ((fmod (target_position.y, cell_size[1]) / cell_size[1]) - 0.5 * ( (0 <= position.y) - (position.y < 0))) * cost_map[cell[0]][cell[1]];

	direction.x += target_position_offset[0];
	direction.y += target_position_offset[1];
	return direction;

}


void FlowField::set_play_field(godot::TileMap* tile_map_s, godot::Dictionary cost_of_tiles_s,
								 int allocation_cells_visited, int allocation_target_cells, int allocation_cells_calculation){
	tile_map = tile_map_s;
	
	godot::Rect2 used_rect = tile_map->get_used_rect();
	godot::Vector2 used_cell_size = tile_map->get_cell_size();
	cost_of_tiles = cost_of_tiles_s;

	cell_size[0] = used_cell_size.x;
	cell_size[1] = used_cell_size.y;

	max_cost_of_a_tile = 2;
	int size_x = used_rect.size.x + buffer_size * 2;
	int size_y = used_rect.size.y + buffer_size * 2;

	offset_x = -used_rect.position.x + buffer_size;
	offset_y = -used_rect.position.y + buffer_size;
	new_offset_x = offset_x;
	new_offset_y = offset_y;

	create_maps(size_x, size_y);
	fill_cost_map();
	all_interesting_cells = std::vector<std::vector<std::vector<int>>>(max_cost_of_a_tile, std::vector<std::vector<int>>(allocation_cells_calculation,std::vector<int>(2,0)));
	cells_visited = std::vector<std::vector<int>>(allocation_cells_visited, std::vector<int>(2,0));
	new_cells_to_consider = std::vector<std::vector<std::vector<int>>>(2,std::vector<std::vector<int>>(allocation_target_cells, std::vector<int>(2,0)));
}


int FlowField::get_cell_distance(godot::Vector2 cell){
	int distance;
	cell.x += offset_x;
	cell.y += offset_y;
	int cell_as_array[2];
	cell_as_array[0] = cell.x;
	cell_as_array[1] = cell.y;

	if (!is_in_play_field(cell_as_array))
	{
		return -1;
	}

		return distance_map[cell.x][cell.y];
}


void FlowField::update_cell_cost(int cell_x, int cell_y, int cell_cost){

	int lowest_x = -new_offset_x;
	int highest_x = distance_map.size() - new_offset_x;
	int lowest_y = -new_offset_y;
	int highest_y = distance_map[0].size() - new_offset_y;

	bool need_resize = false;
	bool data_transfer = false;

	int cell_as_array[2];
	cell_as_array[0] = cell_x + offset_x;
	cell_as_array[1] = cell_y + offset_y;

	if (cell_x > highest_x) {
		highest_x = cell_x;
		need_resize = true;
		
	}
	if (cell_x < lowest_x) {
		lowest_x = cell_x;
		new_offset_x = -cell_x + buffer_size;
		need_resize = true;
		data_transfer = true;
	}
	if (cell_y > highest_y) {
		highest_y = cell_y;
		need_resize = true;
		
	}
	if (cell_y < lowest_y) {
		lowest_y = cell_y;
		new_offset_y = -cell_y + buffer_size;
		need_resize = true;
		data_transfer = true;
	}

	godot::Vector2 cell(cell_x, cell_y);


	special_cost_cells[cell] = cell_cost;


	int tile_index = tile_map->get_cell(cell.x, cell.y);
	if(tile_index<0){
		changed_costs[cell] = cell_cost;
	}
	else {
		godot::Dictionary tmp;
		tmp = cost_of_tiles[tile_index];
		int cost_of_the_tile = tmp["cost"];

	
		if(cell_cost != cost_of_the_tile){
			changed_costs[cell] = cell_cost;
		}
		else {
			changed_costs.erase(cell);

		}

	}

	

	if(need_resize){
		new_size_x = (highest_x - lowest_x) + 1 + buffer_size * 2;
		new_size_y = (highest_y - lowest_y) + 1 + buffer_size * 2;
		expand_maps(new_size_x, new_size_y);
		std::cout << "New dimension: " << new_size_x << ", " << new_size_y;
	}

	if(data_transfer) {
		std::cout << "Offset off Flowfield changed, changes in the offset are cost intensive and should be avoided for maps bigger than 200 cells in each direction \n";
		transfer_data();
		offset_x = new_offset_x;
		offset_y = new_offset_y;
	}


	cost_map_needs_update = true;
}


void FlowField::create_flow_field(godot::Array start_positions, godot::Array target_positions, int safety_margin) {
	if(cost_map_needs_update){
		update_cost_map();
		cost_map_needs_update = false;
	}
	reset_maps();
	count_cells_visited = 0;
	index_to_target_cell = std::vector<std::vector<int>>();

	current_cost_pointer = 0;
	nr_of_cells = std::vector<int>(max_cost_of_a_tile, 0);

	
	interesting_cells_left = 0;
	
	find_start_cells(start_positions);
	
	target_cells_left = 0;
	find_target_cells(target_positions, safety_margin);
	
	// No target positions were given
	if (target_cells_left == 0)
	{
		target_cells_left = 1;
	}
	
	

	int current_nr_of_cells_to_check = 0;
	while (interesting_cells_left > 0 && target_cells_left > 0)
	{

		current_nr_of_cells_to_check = nr_of_cells[current_cost_pointer];
		visit_current_cost_cells(all_interesting_cells[current_cost_pointer], current_nr_of_cells_to_check);
		current_cost_pointer = (current_cost_pointer + 1) % max_cost_of_a_tile;

	}



	if (target_cells_left > 0)
	{
		reset_target_cells(target_positions);
	}

}



void FlowField::reduce_map_size(){
	godot::Rect2 used_rect = tile_map->get_used_rect();
	int lowest_x = used_rect.position.x;
	int highest_x = used_rect.position.x + used_rect.size.x;

	

	int lowest_y = used_rect.position.y;
	int highest_y = used_rect.position.y + used_rect.size.y;

	godot::Array keys = changed_costs.keys();
	for (int i = 0; i < keys.size(); i++)
	{
		godot::Vector2 cell = keys[i];
		if(cell.x < lowest_x){
			lowest_x = cell.x;
		} 
		if(cell.y < lowest_y){
			lowest_y = cell.y;
		} 
		if(cell.x > highest_x){
			highest_x = cell.x;
		} 
		if(cell.y > highest_y){
			highest_y = cell.y;
		} 
	}
	


	max_cost_of_a_tile = 2;
	int size_x = (highest_x - lowest_x) + buffer_size * 2;
	int size_y = (highest_y - lowest_y) + buffer_size * 2;

	offset_x = -lowest_x + buffer_size;
	offset_y = -lowest_y + buffer_size;
	new_offset_x = offset_x;
	new_offset_y = offset_y;

	create_maps(size_x, size_y);
	fill_cost_map();

	for (int i = 0; i < keys.size(); i++)
	{
		godot::Vector2 cell = keys[i];
		int cost_of_cell = changed_costs[cell];
		int cell_as_array[2] = {cell.x + offset_x, cell.y + offset_y};
		cost_map[cell_as_array[0]][cell_as_array[1]] =cost_of_cell;
	}



}

void FlowField::reset_cost_map(){
	godot::Array keys = changed_costs.keys();
	for (int i = 0; i < keys.size(); i++)
	{
		godot::Vector2 cell = keys[i];
		int cell_as_array[2] = {cell.x + offset_x, cell.y + offset_y};
		godot::Dictionary tmp = cost_of_tiles[tile_map->get_cell(cell.x, cell.y)];
		int cost_of_the_tile = tmp["cost"];
		// does not exist
		if (cost_of_the_tile == 0)
		{
			cost_of_the_tile = wall_cost;
		}
		
		cost_map[cell_as_array[0]][cell_as_array[1]] = cost_of_the_tile;

	}



	special_cost_cells = godot::Dictionary();
	changed_costs = godot::Dictionary();
	keys = changed_costs.keys();
	keys = special_cost_cells.keys();
	
}

void FlowField::set_buffer_size(int new_buffer_size) {
	buffer_size = new_buffer_size;
}


void FlowField::reset_maps(){
	for (int i = 0; i < count_cells_visited; i++)
	{
		distance_map[cells_visited[i][0]][cells_visited[i][1]] = max_distance;
	}
	

	/*
	for (size_t i = 0; i < distance_map.size(); i++)
	{
		for (size_t j = 0; j < distance_map[i].size(); j++)
		{
			distance_map[i][j] = max_distance;
			target_cells_saved[i][j] = -1;
			target_visible[i][j] = false;
			target_visible_transferable[i][j] = false;
		}
	}
	*/
}

void FlowField::find_start_cells(godot::Array &start_cells){
	int index = 0;
	for (int i = 0; i < start_cells.size(); i++)
	{
		godot::Vector2 cell = start_cells[i];
		cell.x += offset_x;
		cell.y += offset_y;
		

		int cell_as_array[2];
		cell_as_array[0] = cell.x;
		cell_as_array[1] = cell.y;

		if (!is_in_play_field(cell_as_array))
		{
			continue;
		}
		distance_map[cell.x][cell.y] = 0;
		std::vector<int> cell_as_vector(cell_as_array, cell_as_array + sizeof cell_as_array / sizeof cell_as_array[0]);
		//cell_as_vector[0] = cell.x;
		//cell_as_vector[1] = cell.y;
		interesting_cells_left++;

		if (interesting_cells_left>all_interesting_cells[0].size())
		{
			all_interesting_cells[0].push_back(cell_as_vector);
		}
		else
		{
			all_interesting_cells[0][interesting_cells_left - 1] = cell_as_vector;
		}
		index_to_target_cell.push_back(cell_as_vector);


		if(count_cells_visited >= cells_visited.size()){
			cells_visited.resize(cells_visited.size() + buffer_size, {0,0});
			
		}
		cells_visited[count_cells_visited] = cell_as_vector;
		count_cells_visited++;


		target_cells_saved[cell_as_array[0]][cell_as_array[1]] = index;
		direction_distance[cell_as_array[0]][cell_as_array[1]][0] = 0;
		direction_distance[cell_as_array[0]][cell_as_array[1]][1] = 0;

		target_visible[cell_as_array[0]][cell_as_array[1]] = true;
		target_visible_transferable[cell_as_array[0]][cell_as_array[1]] = true;

		index++;
		
		
	}
	nr_of_cells[0] = interesting_cells_left;
}

void FlowField::find_target_cells(godot::Array &target_cells, int safety_margin){

	// contains the list twice so that we can use the first one and save into the second one or vice versa
	
	int index_to_save = 0;
	int index_to_read = 1;
	int count_new_cells_to_consider = 0;
	for (int i = 0; i < target_cells.size(); i++)
	{
		godot::Vector2 cell = target_cells[i];
		cell.x += offset_x;
		cell.y += offset_y;

		int cell_as_array[2];
		cell_as_array[0] = cell.x;
		cell_as_array[1] = cell.y;

		if (!is_in_play_field(cell_as_array))
		{
			continue;
		}
		if (cost_map[cell_as_array[0]][cell_as_array[1]] == wall_cost){
			continue;
		}

		if (!cells_to_visit[cell.x][cell.y])
		{
			cells_to_visit[cell.x][cell.y] = true;
			new_cells_to_consider[index_to_save][count_new_cells_to_consider][0] = cell.x;
			new_cells_to_consider[index_to_save][count_new_cells_to_consider][1] = cell.y;
			count_new_cells_to_consider += 1;
			if(count_new_cells_to_consider >= new_cells_to_consider[0].size()){
				new_cells_to_consider[0].resize(new_cells_to_consider[0].size() + buffer_size, {0,0});
				new_cells_to_consider[1].resize(new_cells_to_consider[1].size() + buffer_size, {0,0});
			}
			target_cells_left += 1;
		}
	}
	int safety_count = 0;
	while(safety_count < safety_margin && count_new_cells_to_consider >0){
		
		int current_count_new_cells_to_consider = count_new_cells_to_consider;
		count_new_cells_to_consider = 0;
		index_to_save = safety_count % 2 == 0;
		index_to_read = safety_count % 2 == 1;
		
		for (int i = 0; i < current_count_new_cells_to_consider; i++)
		{
			int target_cell_as_array[2] = {new_cells_to_consider[index_to_read][i][0], new_cells_to_consider[index_to_read][i][1]};
			for (int j = 0; j < 4; j++)
			{
				int cell_as_array[2] = {target_cell_as_array[0] + four_directions[j][0], target_cell_as_array[1] + four_directions[j][1]};

				if (!is_in_play_field(cell_as_array))
			{
				continue;
			}
			if (cost_map[cell_as_array[0]][cell_as_array[1]] == wall_cost){
				continue;
				}
				if (!cells_to_visit[cell_as_array[0]][cell_as_array[1]])
				{
					cells_to_visit[cell_as_array[0]][cell_as_array[1]] = true;
					new_cells_to_consider[index_to_save][count_new_cells_to_consider][0] = cell_as_array[0];
					new_cells_to_consider[index_to_save][count_new_cells_to_consider][1] = cell_as_array[1];
					count_new_cells_to_consider += 1;
					if(count_new_cells_to_consider >= new_cells_to_consider[0].size()){
						new_cells_to_consider[0].resize(new_cells_to_consider[0].size() + buffer_size, {0,0});
						new_cells_to_consider[1].resize(new_cells_to_consider[1].size() + buffer_size, {0,0});
					}
					target_cells_left += 1;
				}
			}
			
		}
		safety_count += 1;
		
	}

}

bool FlowField::both_sides_free(std::vector<int> &interesting_cell, std::vector<int> &direction){
	neighbor_x[0] = interesting_cell[0] + direction[0];
	neighbor_x[1] = interesting_cell[1];

	if (!is_in_play_field(neighbor_x))
	{
		return false;
	}
	if (cost_map[neighbor_x[0]][neighbor_x[1]] == wall_cost){
		return false;
	}
	neighbor_y[0] = interesting_cell[0];
	neighbor_y[1] = interesting_cell[1] + direction[1];

}

void FlowField::update_direction_distance(std::vector<int> &interesting_cell, int &new_distance_to_cell, int &direction_index){
	distance_map[neighbor[0]][neighbor[1]] = new_distance_to_cell;


	int new_direction_distance_x = direction_distance[interesting_cell[0]][interesting_cell[1]][0] - (four_directions[direction_index][0] * cost_map[neighbor[0]][neighbor[1]]);
	int new_direction_distance_y = direction_distance[interesting_cell[0]][interesting_cell[1]][1] - (four_directions[direction_index][1] * cost_map[neighbor[0]][neighbor[1]]);
	int direction_step_x = (0 < new_direction_distance_x) - (new_direction_distance_x < 0);
	int direction_step_y = (0 < new_direction_distance_y) - (new_direction_distance_y < 0);


	// straight direction
	if (direction_step_x == 0 || direction_step_y == 0){
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		target_visible[neighbor[0]][neighbor[1]] = target_visible_transferable[interesting_cell[0]][interesting_cell[1]];
		target_visible_transferable[neighbor[0]][neighbor[1]] = target_visible_transferable[interesting_cell[0]][interesting_cell[1]];
		return;
	}
	
	neighbor_x[0] = neighbor[0] + direction_step_x;
	neighbor_x[1] = neighbor[1];
	
	// neighbor in x direction does not exist
	if (!is_in_play_field(neighbor_x)){
		direction_distance[neighbor[0]][neighbor[1]][0] = 0;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		target_visible[neighbor[0]][neighbor[1]] = false;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
		return;
	}

		// neighbor in x direction was not visited
	if(distance_map[neighbor_x[0]][neighbor_x[1]] > new_distance_to_cell){
		direction_distance[neighbor[0]][neighbor[1]][0] = 0;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		target_visible[neighbor[0]][neighbor[1]] = false;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
		return;
	}

	neighbor_y[0] = neighbor[0];
	neighbor_y[1] = neighbor[1] + direction_step_y;

	// neighbor in y direction does not exist
	if (!is_in_play_field(neighbor_y)){
		direction_distance[neighbor[0]][neighbor[1]][1] = 0;
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		target_visible[neighbor[0]][neighbor[1]] = false;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
		return;
		
	}

	// neighbor in y direction was not visited
	 if(distance_map[neighbor_y[0]][neighbor_y[1]] > new_distance_to_cell){
		direction_distance[neighbor[0]][neighbor[1]][1] = 0;
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		target_visible[neighbor[0]][neighbor[1]] = false;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
		return;
	}

	bool neighbor_x_visible_transferable = target_visible_transferable[neighbor_x[0]][neighbor_x[1]];
	bool neighbor_y_visible_transferable = target_visible_transferable[neighbor_y[0]][neighbor_y[1]];

	if (neighbor_x_visible_transferable + neighbor_y_visible_transferable == 1){
		target_visible[neighbor[0]][neighbor[1]] = true;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
	}

	else if(neighbor_x_visible_transferable && neighbor_y_visible_transferable){
		target_visible[neighbor[0]][neighbor[1]] = true;
		target_visible_transferable[neighbor[0]][neighbor[1]] = true;
	}

	else {
		target_visible[neighbor[0]][neighbor[1]] = false;
		target_visible_transferable[neighbor[0]][neighbor[1]] = false;
	}
	// neighbor in x and y direction were already visited and we want to create a new vector based on the two
	if(abs(direction_distance[neighbor_y[0]][neighbor_y[1]][0]) < abs(direction_distance[neighbor_x[0]][neighbor_x[1]][0])){

		direction_distance[neighbor[0]][neighbor[1]][0] = direction_distance[neighbor_y[0]][neighbor_y[1]][0] + direction_step_x * cost_map[neighbor[0]][neighbor[1]];
	}
	else {
		direction_distance[neighbor[0]][neighbor[1]][0] = direction_distance[neighbor_x[0]][neighbor_x[1]][0] + direction_step_x * cost_map[neighbor[0]][neighbor[1]];
	}


	if(abs(direction_distance[neighbor_y[0]][neighbor_y[1]][1]) < abs(direction_distance[neighbor_x[0]][neighbor_x[1]][1])){

		direction_distance[neighbor[0]][neighbor[1]][1] = direction_distance[neighbor_y[0]][neighbor_y[1]][1] + direction_step_y * cost_map[neighbor[0]][neighbor[1]];
	}
	else {

		direction_distance[neighbor[0]][neighbor[1]][1] = direction_distance[neighbor_x[0]][neighbor_x[1]][1] + direction_step_y * cost_map[neighbor[0]][neighbor[1]];
	} 
	
}

void FlowField::visit_neighbor(std::vector<int> &interesting_cell, int direction_index){
	neighbor[0] = interesting_cell[0] + four_directions[direction_index][0];
	neighbor[1] = interesting_cell[1] + four_directions[direction_index][1];



	if (!is_in_play_field(neighbor))
	{
		return;
	}

	int cell_cost = cost_map[neighbor[0]][neighbor[1]];

	if (cell_cost == wall_cost){
		return;
	}

	int new_distance_to_cell = distance_map[interesting_cell[0]][interesting_cell[1]] + cell_cost;
	int new_distance_as_cost_pointer = new_distance_to_cell % max_cost_of_a_tile;
	int old_distance = distance_map[neighbor[0]][neighbor[1]];

	if (new_distance_to_cell < old_distance){
		if (old_distance == max_distance){
			if(count_cells_visited >= cells_visited.size()){
				cells_visited.resize(cells_visited.size() + buffer_size, {0,0});
			}
			
			cells_visited[count_cells_visited][0] = neighbor[0];
			cells_visited[count_cells_visited][1] = neighbor[1];
			count_cells_visited++;
		}

		//int new_distance_as_cost_pointer = new_distance_to_cell % max_cost_of_a_tile;
		interesting_cells_left += 1;

		target_cells_saved[neighbor[0]][neighbor[1]] = target_cells_saved[interesting_cell[0]][interesting_cell[1]];
		
		update_direction_distance(interesting_cell, new_distance_to_cell, direction_index);


		if (nr_of_cells[new_distance_as_cost_pointer] >= all_interesting_cells[new_distance_as_cost_pointer].size())
		{
			all_interesting_cells[new_distance_as_cost_pointer].resize(all_interesting_cells[new_distance_as_cost_pointer].size() + buffer_size, {0,0});
		}

		
		all_interesting_cells[new_distance_as_cost_pointer][nr_of_cells[new_distance_as_cost_pointer]][0] = neighbor[0];
		all_interesting_cells[new_distance_as_cost_pointer][nr_of_cells[new_distance_as_cost_pointer]][1] = neighbor[1];
		nr_of_cells[new_distance_as_cost_pointer] += 1;

	}
}

void FlowField::visit_neighbors(std::vector<int> &interesting_cell){
		if (cells_to_visit[interesting_cell[0]][interesting_cell[1]])
		{
			cells_to_visit[interesting_cell[0]][interesting_cell[1]] = false;
			target_cells_left--;
		}


		for (int k = 0; k < four_directions.size(); k++){
			visit_neighbor(interesting_cell, k);
		}


}

void FlowField::visit_current_cost_cells(std::vector<std::vector<int>> &interesting_cells, int &interesting_cells_size){
	for (int i = 0; i < interesting_cells_size; i++)
	{
		interesting_cells_left -= 1;
		visit_neighbors(interesting_cells[i]);
	}
	nr_of_cells[current_cost_pointer] = 0;
}



void FlowField::reset_target_cells(godot::Array &target_cells){
	for (int i = 0; i < target_cells.size(); i++)
	{
		godot::Vector2 cell = target_cells[i];
		cell.x += offset_x;
		cell.y += offset_y;

		int cell_as_array[2];
		cell_as_array[0] = cell.x;
		cell_as_array[1] = cell.y;

		if (!is_in_play_field(cell_as_array))
		{
			continue;
		}
		cells_to_visit[cell.x][cell.y] = true;
	}
}

bool FlowField::is_in_play_field(std::vector<int> cell){
		if ( cell[0]>= distance_map.size() || cell[0]<0)
	{
		return false;
	}

	if ( cell[1]<0 || cell[1] >= distance_map[cell[0]].size())
	{
		return false;
	}
	
	return true;


}

bool FlowField::is_in_play_field(int cell[]){
		if (cell[0]>= distance_map.size() || cell[0]<0)
	{
		return false;
	}

	if ( cell[1]<0 || cell[1] >= distance_map[cell[0]].size())
	{
		return false;
	}
	
	return true;
}


void FlowField::transfer_data(){
	int difference_x = new_offset_x - offset_x;
	int difference_y = new_offset_y - offset_y;

	
	// take information from old positions to new position, going from end to beginning to avoid overwritting relevant data (difference_x always positive)
	for (int i = cost_map.size() - difference_x - 1; i >= 0 ; i--)
	{
		
		for (int j = cost_map[i].size() - difference_y - 1; j >= 0; j--)
		{

			cost_map[i + difference_x][j + difference_y] = cost_map[i][j];
		}
		
	}
	
}

void FlowField::expand_maps(int target_size_x, int target_size_y){

	for (int i = 0; i < distance_map.size(); i++)
	{
		int empty[2];
		distance_map[i].resize(target_size_y, max_distance);
		target_cells_saved[i].resize(target_size_y, -1);
		target_visible[i].resize(target_size_y, false);
		target_visible_transferable[i].resize(target_size_y, false);
		cost_map[i].resize(target_size_y, wall_cost);
		direction_distance[i].resize(target_size_y, {0,0});
		cells_to_visit[i].resize(target_size_y, false);
	}

	distance_map.resize(target_size_x, std::vector<int>(target_size_y, max_distance));
	target_cells_saved.resize(target_size_x, std::vector<int>(target_size_y, -1));
	target_visible.resize(target_size_x, std::vector<bool>(target_size_y, false));
	target_visible_transferable.resize(target_size_x, std::vector<bool>(target_size_y, false));
	cost_map.resize(target_size_x, std::vector<int16_t>(target_size_y, wall_cost));
	direction_distance.resize(target_size_x, std::vector<std::vector<int>>(target_size_y, std::vector<int>(2, 0)));
	cells_to_visit.resize(target_size_x,  std::vector<bool>(target_size_y, false));
	
}

void FlowField::update_cost_map(){
	godot::Array keys = special_cost_cells.keys();

	for(int i=0; i<keys.size(); i++){
		godot::Vector2 cell = keys[i];
		int cell_as_array[2];
		cell_as_array[0] = cell.x + offset_x;
		cell_as_array[1] = cell.y + offset_y;

		if(!is_in_play_field(cell_as_array)){
			continue;
		}
		int cost = special_cost_cells[cell];
		cost_map[cell_as_array[0]][cell_as_array[1]] = cost;
		special_cost_cells.erase(cell);
	}
}

void FlowField::create_maps(int size_x, int size_y){
	distance_map = std::vector<std::vector<int>>(size_x, std::vector<int>(size_y,max_distance));
	target_cells_saved =std::vector<std::vector<int>>(size_x, std::vector<int>(size_y,-1));
	target_visible = std::vector<std::vector<bool>>(size_x, std::vector<bool>(size_y,false));
	target_visible_transferable = std::vector<std::vector<bool>>(size_x, std::vector<bool>(size_y,false));


	direction_distance = std::vector<std::vector<std::vector<int>>>(size_x, std::vector<std::vector<int>>(size_y, std::vector<int>(2,0)));

	cost_map = std::vector<std::vector<int16_t>>(size_x, std::vector<int16_t>(size_y,wall_cost));
	cells_to_visit = std::vector<std::vector<bool>>(size_x, std::vector<bool>(size_y,false));
}

void FlowField::fill_cost_map(){
	godot::Array used_cells = tile_map->get_used_cells();
	int cost_of_the_tile = 0;
	godot::Dictionary tmp;

	for (int i = 0; i <used_cells.size(); i++)
	{
		tmp = cost_of_tiles[tile_map->get_cell(godot::Vector2(used_cells[i]).x, godot::Vector2(used_cells[i]).y)];
		cost_of_the_tile = tmp["cost"];

		if ((cost_of_the_tile+1) > max_cost_of_a_tile)
		{
			max_cost_of_a_tile = cost_of_the_tile + 1;
		}
		

		if (cost_of_the_tile == 0)
		{
			cost_of_the_tile = wall_cost;
		}
		
		
		cost_map[offset_x + godot::Vector2(used_cells[i]).x][offset_y + godot::Vector2(used_cells[i]).y] = cost_of_the_tile;
		
	}

}








