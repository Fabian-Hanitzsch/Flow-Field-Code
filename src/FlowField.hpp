#ifndef FlowField_H
#define FlowField_H

#include <Node2D.hpp>
#include <TileMap.hpp>
#include <Godot.hpp>
#include <vector>

#include <unordered_set>
#include <iterator>

class FlowField : public godot::Node2D {
	const std::vector<std::vector<int>> four_directions = {{1,0}, {-1,0}, {0,1}, {0,-1}};


	GODOT_CLASS(FlowField, godot::Node2D)
	bool cost_map_needs_update;
	std::vector<std::vector<int>> distance_map;
	std::vector<std::vector<int16_t>> cost_map;
	godot::Dictionary special_cost_cells;


	std::vector<std::vector<bool>> target_visible;
	std::vector<std::vector<bool>> target_visible_transferable;

	std::vector<std::vector<int>> index_to_target_cell;

	std::vector<std::vector<int>> target_cells_saved;
	


	std::vector<std::vector<std::vector<int>>> direction_distance;
	std::vector<std::vector<std::vector<int>>> new_cells_to_consider;

	godot::Dictionary changed_costs;

	std::vector<std::vector<bool>> cells_to_visit;
	std::vector<std::vector<int>> cells_visited;
	int count_cells_visited = 0;
	int current_cost_pointer = 0;

	int neighbor[2];
	int neighbor_x[2];
	int neighbor_y[2];
	std::vector<int> nr_of_cells;
	std::vector<std::vector<std::vector<int>>> all_interesting_cells;
	int interesting_cells_left = 0;
	int target_cells_left = 0;
	int cell_size[2];
	int buffer_size = 0;
	godot::Dictionary cost_of_tiles;

	int new_size_x = 0;
	int new_size_y = 0;
	int new_offset_x = 0;
	int new_offset_y = 0;


	const int16_t wall_cost = 255;
	const int max_distance = 100000;
	int max_cost_of_a_tile = 2;
	int offset_x = 0;
	int offset_y = 0;
	godot::TileMap* tile_map;

public:
	void _init() {}
	godot::Array get_direction_map();
	godot::Vector2 get_cell_direction(godot::Vector2);

	int get_cell_distance(godot::Vector2);
	void create_flow_field(godot::Array, godot::Array, int);
	static void _register_methods();
	void set_play_field(godot::TileMap*, godot::Dictionary, int, int, int);

	godot::Vector2 FlowField::get_direction(godot::Vector2, bool, godot::Vector2);

	void FlowField::update_cell_cost(int cell_x, int cell_y, int cell_cost);
	void FlowField::set_buffer_size(int);
	void FlowField::reduce_map_size();
	void FlowField::reset_cost_map();
	
	
	

private:
	bool is_in_play_field(int[]);
	bool is_in_play_field(std::vector<int>);
	void reset_maps();
	void find_start_cells(godot::Array&);
	void find_target_cells(godot::Array&, int);
	void visit_current_cost_cells(std::vector<std::vector<int>>&, int&);
	void FlowField::reset_target_cells(godot::Array&);
	void FlowField::visit_neighbors(std::vector<int>&);
	void FlowField::visit_neighbor(std::vector<int>&, int);
	bool FlowField::both_sides_free(std::vector<int>&, std::vector<int>&);
	void FlowField::update_direction_distance(std::vector<int>&, int&, int&);
	void FlowField::create_maps(int, int);
	void FlowField::update_cost_map();
	void FlowField::fill_cost_map();
	void FlowField::expand_maps(int, int);
	void FlowField::transfer_data();
	
	
};




#endif // FlowField_H
