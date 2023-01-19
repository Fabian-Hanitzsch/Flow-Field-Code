#ifndef FlowField_H
#define FlowField_H

#include <Node2D.hpp>
#include <TileMap.hpp>
#include <Godot.hpp>
#include <vector>

class FlowField : public godot::Node2D {
	const std::vector<std::vector<int>> four_directions = {{1,0}, {-1,0}, {0,1}, {0,-1}};
	const std::vector<std::vector<int>> four_diagonals = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};

	GODOT_CLASS(FlowField, godot::Node2D)
	std::vector<std::vector<int>> distance_map;
	std::vector<std::vector<int>> cost_map;
	std::vector<std::vector<std::vector<int>>> direction_map;
	std::vector<std::vector<std::vector<int>>> direction_distance;
	std::vector<std::vector<int>> process_number_of_cell;
	std::vector<std::vector<bool>> cells_to_visit;
	int current_cost_pointer = 0;
	int process_number = 0;
	int neighbor[2];
	int neighbor_x[2];
	int neighbor_y[2];
	std::vector<int> nr_of_cells;
	std::vector<std::vector<std::vector<int>>> all_interesting_cells;
	int interesting_cells_left = 0;
	int target_cells_left = 0;


	const int wall_cost = 255;
	const int max_distance = 65535;
	int max_cost_of_a_tile = 2;
	int offset_x = 0;
	int offset_y = 0;

public:
	void _init() {}
	godot::Array get_distance_map();
	godot::Array get_direction_map();
	godot::Vector2 get_cell_direction(godot::Vector2);
	godot::Vector2 get_exact_cell_direction(godot::Vector2);
	int get_cell_distance(godot::Vector2);
	void create_flow_field(godot::Array start_cells, godot::Array target_cells, bool diagonals_allowed, bool diagonals_need_neighbors);
	static void _register_methods();
	void set_play_field(godot::TileMap*, godot::Dictionary);
	int FlowField::get_process_number_of_cell(godot::Vector2);
	

private:
	bool is_in_play_field(int[]);
	bool is_in_play_field(std::vector<int>);
	void reset_maps();
	void find_start_cells(godot::Array&);
	void find_target_cells(godot::Array&);
	void visit_current_cost_cells(std::vector<std::vector<int>>&, int&);
	void FlowField::reset_target_cells(godot::Array&);
	void FlowField::visit_neighbors(std::vector<int>&);
	void FlowField::visit_neighbor(std::vector<int>&, int);
	bool FlowField::both_sides_free(std::vector<int>&, std::vector<int>&);
	void FlowField::update_direction_distance(std::vector<int>&, int&, int&);
};




#endif // FlowField_H
