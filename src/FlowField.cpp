#include "FlowField.hpp"




void FlowField::_register_methods() {
	godot::register_method("get_distance_map", &FlowField::get_distance_map);
	godot::register_method("set_play_field", &FlowField::set_play_field);
	godot::register_method("create_flow_field", &FlowField::create_flow_field);
	godot::register_method("get_direction_map", &FlowField::get_direction_map);
	godot::register_method("get_cell_direction", &FlowField::get_cell_direction);
	godot::register_method("get_exact_cell_direction", &FlowField::get_exact_cell_direction);
	godot::register_method("get_cell_distance", &FlowField::get_cell_distance);
	godot::register_method("get_process_number_of_cell", &FlowField::get_process_number_of_cell);

}



godot::Array FlowField::get_direction_map () {
	godot::Array ret;

	for (size_t i = 0; i < direction_map.size(); i++)
	{
		godot::Array row;
		for (size_t j = 0; j < direction_map[i].size(); j++)
		{
			godot::Vector2 cell;
			cell.x = direction_map[i][j][0];
			cell.y = direction_map[i][j][1];
			row.append(cell);
		}
		ret.append(row);
		
	}

	return ret;
}

godot::Array FlowField::get_distance_map () {
	godot::Array ret;
	for (size_t i = 0; i < distance_map.size(); i++)
	{
		godot::Array row;
		for (size_t j = 0; j < distance_map[i].size(); j++)
		{
			row.append(distance_map[i][j]);
		}
		ret.append(row);
		
	}

	return ret;
}


void FlowField::reset_maps(){
	for (size_t i = 0; i < distance_map.size(); i++)
	{
		for (size_t j = 0; j < distance_map[i].size(); j++)
		{
			distance_map[i][j] = max_distance;
			//direction_distance[i][j][0] = 0;
			//direction_distance[i][j][1] = 0;
		}
	}
}

void FlowField::find_start_cells(godot::Array &start_cells){
	for (size_t i = 0; i < start_cells.size(); i++)
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
		std::vector<int> cell_as_vector = std::vector<int>(2,0);
		cell_as_vector[0] = cell.x;
		cell_as_vector[1] = cell.y;

		if (interesting_cells_left>=all_interesting_cells[0].size())
		{
			all_interesting_cells[0].push_back(cell_as_vector);
		}
		else
		{
			all_interesting_cells[0][interesting_cells_left] = cell_as_vector;
		}
		interesting_cells_left++;
	}
	nr_of_cells[0] = interesting_cells_left;
}

void FlowField::find_target_cells(godot::Array &target_cells){
	for (size_t i = 0; i < target_cells.size(); i++)
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
		if (!cells_to_visit[cell.x][cell.y])
		{
			cells_to_visit[cell.x][cell.y] = true;
			target_cells_left += 1;
		}
		
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
	process_number_of_cell[neighbor[0]][neighbor[1]] = process_number;
	process_number++;
	//direction_map[neighbor[0]][neighbor[1]][0] =  interesting_cell[0]  - neighbor[0];
	//direction_map[neighbor[0]][neighbor[1]][1] =  interesting_cell[1]  - neighbor[1];

	int new_direction_distance_x = direction_distance[interesting_cell[0]][interesting_cell[1]][0] - four_directions[direction_index][0];
	int new_direction_distance_y = direction_distance[interesting_cell[0]][interesting_cell[1]][1] - four_directions[direction_index][1];
	int direction_step_x = (0 < new_direction_distance_x) - (new_direction_distance_x < 0);
	int direction_step_y = (0 < new_direction_distance_y) - (new_direction_distance_y < 0);


	// straight direction
	if (direction_step_x == 0 || direction_step_y == 0){
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		return;
	}
	
	neighbor_x[0] = neighbor[0] + direction_step_x;
	neighbor_x[1] = neighbor[1];
	
	// neighbor in x direction does not exist
	if (!is_in_play_field(neighbor_x)){
		direction_distance[neighbor[0]][neighbor[1]][0] = 0;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		return;
	}

		// neighbor in x direction was not visited
	if(distance_map[neighbor_x[0]][neighbor_x[1]] > new_distance_to_cell){
		direction_distance[neighbor[0]][neighbor[1]][0] = 0;
		direction_distance[neighbor[0]][neighbor[1]][1] = new_direction_distance_y;
		return;
	}

	neighbor_y[0] = neighbor[0];
	neighbor_y[1] = neighbor[1] + direction_step_y;

	// neighbor in y direction does not exist
	if (!is_in_play_field(neighbor_y)){
		direction_distance[neighbor[0]][neighbor[1]][1] = 0;
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		return;
		
	}

	// neighbor in y direction was not visited
	 if(distance_map[neighbor_y[0]][neighbor_y[1]] > new_distance_to_cell){
		direction_distance[neighbor[0]][neighbor[1]][1] = 0;
		direction_distance[neighbor[0]][neighbor[1]][0] = new_direction_distance_x;
		return;
	}

	// neighbor in x and y direction were already visited and we want to create a new vector based on the two
	if(abs(direction_distance[neighbor_y[0]][neighbor_y[1]][0]) < abs(direction_distance[neighbor_x[0]][neighbor_x[1]][0])){

		direction_distance[neighbor[0]][neighbor[1]][0] = direction_distance[neighbor_y[0]][neighbor_y[1]][0] + direction_step_x;
	}
	else {
		direction_distance[neighbor[0]][neighbor[1]][0] = direction_distance[neighbor_x[0]][neighbor_x[1]][0] + direction_step_x;
	}


	if(abs(direction_distance[neighbor_y[0]][neighbor_y[1]][1]) < abs(direction_distance[neighbor_x[0]][neighbor_x[1]][1])){

		direction_distance[neighbor[0]][neighbor[1]][1] = direction_distance[neighbor_y[0]][neighbor_y[1]][1] + direction_step_y;
	}
	else {

		direction_distance[neighbor[0]][neighbor[1]][1] = direction_distance[neighbor_x[0]][neighbor_x[1]][1] + direction_step_y;
	} 
	
}

void FlowField::visit_neighbor(std::vector<int> &interesting_cell, int direction_index){
	neighbor[0] = interesting_cell[0] + four_directions[direction_index][0];
	neighbor[1] = interesting_cell[1] + four_directions[direction_index][1];

	if (!is_in_play_field(neighbor))
	{
		return;
	}

	if (cost_map[neighbor[0]][neighbor[1]] == wall_cost){
		return;
	}

	//bool diagonal = abs(direction[0]) + abs(direction[1]) == 2;
	//if (diagonal && !both_sides_free(interesting_cell, direction)){
	//	return;
	//}

	int new_distance_to_cell = distance_map[interesting_cell[0]][interesting_cell[1]] + cost_map[neighbor[0]][neighbor[1]];
	if (new_distance_to_cell < distance_map[neighbor[0]][neighbor[1]]){
		int new_distance_as_cost_pointer = new_distance_to_cell % max_cost_of_a_tile;
		nr_of_cells[new_distance_as_cost_pointer] += 1;
		interesting_cells_left += 1;
		if (cells_to_visit[neighbor[0]][neighbor[1]])
		{
			cells_to_visit[neighbor[0]][neighbor[1]] = false;
			target_cells_left--;
		}
		
		update_direction_distance(interesting_cell, new_distance_to_cell, direction_index);


		if (nr_of_cells[new_distance_as_cost_pointer] >= all_interesting_cells[new_distance_as_cost_pointer].size())
		{
			all_interesting_cells[new_distance_as_cost_pointer].resize(all_interesting_cells[new_distance_as_cost_pointer].size() * 2, {0,0});
		}
		all_interesting_cells[new_distance_as_cost_pointer][nr_of_cells[new_distance_as_cost_pointer]][0] = neighbor[0];
		all_interesting_cells[new_distance_as_cost_pointer][nr_of_cells[new_distance_as_cost_pointer]][1] = neighbor[1];

		//direction_map[neighbor[0]][neighbor[1]][0] = interesting_cell[0]  - neighbor[0];
		//direction_map[neighbor[0]][neighbor[1]][1] = interesting_cell[1]  - neighbor[1];
	}
}

void FlowField::visit_neighbors(std::vector<int> &interesting_cell){
		for (size_t k = 0; k < four_directions.size(); k++){
			visit_neighbor(interesting_cell, k);
		}
	//	for (size_t k = 0; k < four_diagonals.size(); k++){
	//		visit_neighbor(interesting_cell, four_diagonals[k]);
	//	}

}

void FlowField::visit_current_cost_cells(std::vector<std::vector<int>> &interesting_cells, int &interesting_cells_size){
	for (size_t i = 0; i <= interesting_cells_size; i++)
	{
		nr_of_cells[current_cost_pointer] -= 1;
		interesting_cells_left -= 1;
		visit_neighbors(interesting_cells[i]);
	}
}



void FlowField::create_flow_field(godot::Array start_positions, godot::Array target_positions, bool diagonals_allowed, bool diagonals_need_neighbors) {
	reset_maps();
	process_number = 0;
	current_cost_pointer = 0;
	nr_of_cells = std::vector<int>(max_cost_of_a_tile, 0);

	all_interesting_cells = std::vector<std::vector<std::vector<int>>>(max_cost_of_a_tile, std::vector<std::vector<int>>(10,std::vector<int>(2,0)));
	interesting_cells_left = 0;
	
	find_start_cells(start_positions);
	
	target_cells_left = 0;
	find_target_cells(target_positions);

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
	std::cout << interesting_cells_left;
	std::cout << "AAAAA";

	if (target_cells_left > 0)
	{
		reset_target_cells(target_positions);
	}
	
	
}

void FlowField::reset_target_cells(godot::Array &target_cells){
	for (size_t i = 0; i < target_cells.size(); i++)
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

void FlowField::set_play_field(godot::TileMap* tile_map, godot::Dictionary cost_of_tiles){
	godot::Rect2 used_rect = tile_map->get_used_rect();
	godot::Array used_cells = tile_map->get_used_cells();

	max_cost_of_a_tile = 2;

	if (used_rect.position.x > (used_rect.position.x + used_rect.size.x)){
		offset_x = -(used_rect.position.x + used_rect.size.x);
	}
	else {
		offset_x = -used_rect.position.x;
	}

	if (used_rect.position.y > (used_rect.position.y + used_rect.size.y)){
		offset_y = -(used_rect.position.y + used_rect.size.y);
	}
	else {
		offset_y = -used_rect.position.y;
	}

	distance_map = std::vector<std::vector<int>>(used_rect.size.x, std::vector<int>(used_rect.size.y,max_distance));
	process_number_of_cell = std::vector<std::vector<int>>(used_rect.size.x, std::vector<int>(used_rect.size.y,0));
	direction_distance = std::vector<std::vector<std::vector<int>>>(used_rect.size.x, std::vector<std::vector<int>>(used_rect.size.y, std::vector<int>(2,0)));
	direction_map = std::vector<std::vector<std::vector<int>>>(used_rect.size.x, std::vector<std::vector<int>>(used_rect.size.y, std::vector<int>(2,0)));

	cost_map = std::vector<std::vector<int>>(used_rect.size.x, std::vector<int>(used_rect.size.y,wall_cost));
	cells_to_visit = std::vector<std::vector<bool>>(used_rect.size.x, std::vector<bool>(used_rect.size.y,false));
	int cost_of_the_tile = 0;
	godot::Dictionary tmp;

	for (size_t i = 0; i <used_cells.size(); i++)
	{
		tmp = cost_of_tiles[tile_map->get_cell(godot::Vector2(used_cells[i]).x, godot::Vector2(used_cells[i]).y)];
		cost_of_the_tile = tmp["cost"];

		if ((cost_of_the_tile+1) > max_cost_of_a_tile)
		{
			max_cost_of_a_tile = cost_of_the_tile + 1;
		}
		

		if (cost_of_the_tile == 0)
		{
			cost_of_the_tile = 255;
		}
		
		
		cost_map[offset_x + godot::Vector2(used_cells[i]).x][offset_y + godot::Vector2(used_cells[i]).y] = cost_of_the_tile;
		
	}


}

godot::Vector2 FlowField::get_exact_cell_direction(godot::Vector2 cell){
	godot::Vector2 direction;
	cell.x += offset_x;
	cell.y += offset_y;
	int cell_as_array[2];
	cell_as_array[0] = cell.x;
	cell_as_array[1] = cell.y;

	if (!is_in_play_field(cell_as_array))
	{
		return direction;
	}
		direction.x = direction_distance[cell.x][cell.y][0];
		direction.y = direction_distance[cell.x][cell.y][1];
		return direction;
}


godot::Vector2 FlowField::get_cell_direction(godot::Vector2 cell){
	godot::Vector2 direction;
	cell.x += offset_x;
	cell.y += offset_y;
	int cell_as_array[2];
	cell_as_array[0] = cell.x;
	cell_as_array[1] = cell.y;

	if (!is_in_play_field(cell_as_array))
	{
		return direction;
	}
		direction.x = direction_map[cell.x][cell.y][0];
		direction.y = direction_map[cell.x][cell.y][1];
		return direction;
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

int FlowField::get_process_number_of_cell(godot::Vector2 cell){
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

		return process_number_of_cell[cell.x][cell.y];
}
