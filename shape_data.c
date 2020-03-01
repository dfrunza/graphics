// -*- coding: utf-8 -*-

#define FONT_NAME "Px437_IBM_ISO8"

//  'A'
#define shape_0_n_contours 4
int shape_0_contours[] = {12, 4, 4, 4, };
Point shape_0_points[] = {
	{0.0f, 0.0f, 1.0f}, {0.0f, 8.0f, 1.0f}, {1.0f, 8.0f, 1.0f}, {1.0f, 5.0f, 1.0f}, {6.0f, 5.0f, 1.0f}, {6.0f, 8.0f, 1.0f}, {7.0f, 8.0f, 1.0f}, {7.0f, 0.0f, 1.0f}, {6.0f, 0.0f, 1.0f}, {6.0f, 4.0f, 1.0f}, {1.0f, 4.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, 

	{1.0f, 8.0f, 1.0f}, {1.0f, 9.0f, 1.0f}, {2.0f, 9.0f, 1.0f}, {2.0f, 8.0f, 1.0f}, 

	{2.0f, 9.0f, 1.0f}, {2.0f, 10.0f, 1.0f}, {5.0f, 10.0f, 1.0f}, {5.0f, 9.0f, 1.0f}, 

	{5.0f, 9.0f, 1.0f}, {6.0f, 9.0f, 1.0f}, {6.0f, 8.0f, 1.0f}, {5.0f, 8.0f, 1.0f}, 

};
#define shape_0_total_point_count 24

//  'B'
#define shape_1_n_contours 3
int shape_1_contours[] = {12, 4, 4, };
Point shape_1_points[] = {
	{0.0f, 0.0f, 1.0f}, {0.0f, 10.0f, 1.0f}, {6.0f, 10.0f, 1.0f}, {6.0f, 9.0f, 1.0f}, {1.0f, 9.0f, 1.0f}, {1.0f, 6.0f, 1.0f}, {6.0f, 6.0f, 1.0f}, {6.0f, 5.0f, 1.0f}, {1.0f, 5.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {6.0f, 1.0f, 1.0f}, {6.0f, 0.0f, 1.0f}, 

	{6.0f, 1.0f, 1.0f}, {6.0f, 5.0f, 1.0f}, {7.0f, 5.0f, 1.0f}, {7.0f, 1.0f, 1.0f}, 

	{6.0f, 6.0f, 1.0f}, {6.0f, 9.0f, 1.0f}, {7.0f, 9.0f, 1.0f}, {7.0f, 6.0f, 1.0f}, 

};
#define shape_1_total_point_count 20

//  '▲'
#define shape_2_n_contours 1
int shape_2_contours[] = {16, };
Point shape_2_points[] = {
	{0.0f, 2.0f, 1.0f}, {0.0f, 4.0f, 1.0f}, {1.0f, 4.0f, 1.0f}, {1.0f, 5.0f, 1.0f}, {2.0f, 5.0f, 1.0f}, {2.0f, 6.0f, 1.0f}, {3.0f, 6.0f, 1.0f}, {3.0f, 7.0f, 1.0f}, {4.0f, 7.0f, 1.0f}, {4.0f, 6.0f, 1.0f}, {5.0f, 6.0f, 1.0f}, {5.0f, 5.0f, 1.0f}, {6.0f, 5.0f, 1.0f}, {6.0f, 4.0f, 1.0f}, {7.0f, 4.0f, 1.0f}, {7.0f, 2.0f, 1.0f}, 

};
#define shape_2_total_point_count 16

//  '■'
#define shape_3_n_contours 1
int shape_3_contours[] = {4, };
Point shape_3_points[] = {
	{1.0f, 1.0f, 1.0f}, {1.0f, 8.0f, 1.0f}, {6.0f, 8.0f, 1.0f}, {6.0f, 1.0f, 1.0f}, 

};
#define shape_3_total_point_count 4

Shape shape_data[] = {
	{
		.character=L'A',
		.contours=shape_0_contours,
		.n_contours=shape_0_n_contours,
		.points=shape_0_points,
		.total_point_count=shape_0_total_point_count,
	}, 
	{
		.character=L'B',
		.contours=shape_1_contours,
		.n_contours=shape_1_n_contours,
		.points=shape_1_points,
		.total_point_count=shape_1_total_point_count,
	}, 
	{
		.character=L'▲',
		.contours=shape_2_contours,
		.n_contours=shape_2_n_contours,
		.points=shape_2_points,
		.total_point_count=shape_2_total_point_count,
	}, 
	{
		.character=L'■',
		.contours=shape_3_contours,
		.n_contours=shape_3_n_contours,
		.points=shape_3_points,
		.total_point_count=shape_3_total_point_count,
	}, 
};
