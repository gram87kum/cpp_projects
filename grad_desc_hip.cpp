#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include<cmath>
#include "hip/hip_runtime.h"
#include <thrust/device_vector.h>
#include <thrust/device_ptr.h>
#include <thrust/device_malloc.h>
#include <thrust/device_free.h>
#include <thrust/host_vector.h>
#include <thrust/random.h>
#include <thrust/transform.h>
#include <thrust/transform_reduce.h>
#include <thrust/functional.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/iterator/constant_iterator.h>

//using namespace std;
using namespace thrust::placeholders;
	
typedef thrust::host_vector<thrust::tuple<float,float>> host_vec_pair;
typedef thrust::device_vector<thrust::tuple<float,float>> dev_vec_pair;
typedef thrust::tuple<float,float> tup_pair;

//float numPoints =1000;



//Generate linear random synthetic dataset 
host_vec_pair  rand_lin_coordinates(const int numPoints) {
	//std::mt19937 rng(42); // Mersenne Twister engine for better randomness
	thrust::default_random_engine rng(42);
	thrust::uniform_real_distribution<double> dist(0, 10); // Adjust range as needed

	host_vec_pair coordinates(numPoints);

	for (size_t i = 0; i < numPoints; ++i) {
		float x = dist(rng);
		//double y = 2 * x + 1 + dist(rng) * 2; // y = 2x + 1 with added noise
		float y = 0.57 * x + 1;
		//std::cout<<"X: "<<x<<"Y: "<<y<<std::endl;;
		coordinates[i] = tup_pair(x, y);
		//std::cout<<"Iter:\t"<<i<<std::endl;;
	}
	return coordinates;

}

//Calculate dot product of tuple pair

struct DotProduct: public thrust::binary_function<tup_pair,tup_pair,float>
{
	__host__ __device__
		float operator()(tup_pair &a, tup_pair &b){
			return thrust::get<0>(a)*thrust::get<0>(b)+
				thrust::get<1>(a)* thrust::get<1>(b);
		}
};

//Transform input coordinate x to (1,x)

struct tf_vector: public thrust::unary_function<tup_pair,tup_pair>
{
	__host__ __device__
		tup_pair operator()(tup_pair &a){
			return tup_pair (1,thrust::get<0>(a));
					}
};


//Strip output coordinate 
struct output_vector:public thrust::unary_function<tup_pair,float>
{
	__host__ __device__
		auto operator()(tup_pair &t){
			return thrust::get<1>(t);
		}
};


struct tuple_subtract:public thrust::binary_function<tup_pair,tup_pair,tup_pair>
{
	__host__ __device__
		tup_pair operator()(tup_pair &a, tup_pair &b){
			return tup_pair (thrust::get<0>(a) - thrust::get<0>(b),thrust::get<1>(a)-thrust::get<1>(b));
		}
};

int main(){
	int numPoints =3;
	std::cout<<"Enter number of points : ";
	std::cin>>numPoints;

	float eta = 0.1;
        std::cout<<"Enter ETA : ";
        std::cin>>eta;

	float init = 0; //Init variable for thrust::transform_reduce
	//Host memory initialization
	host_vec_pair h_coordinates = rand_lin_coordinates(numPoints);
	//
	//host_vec_pair h_coordinates = {tup_pair(1,1),tup_pair(2,3),tup_pair(4,3)};


	//Device memory initialization
	dev_vec_pair d_coordinates(numPoints);
	d_coordinates = h_coordinates;
	dev_vec_pair input_A(numPoints);
	thrust::device_vector<float> output_A(numPoints);
	//thrust::host_vector<float> h_output_A(numPoints);
	thrust::device_vector<float> grad_desc_vec(numPoints);
	dev_vec_pair d_weights(numPoints);
	dev_vec_pair d_weights_new(numPoints);
	dev_vec_pair d_weights_temp(numPoints);
	thrust::device_vector<float> result(numPoints);
	tup_pair n_weights{};
	tup_pair weights = thrust::make_tuple(0.0,0.0);
	thrust::fill(d_weights.begin(),d_weights.end(),weights); // Initialize vector of weights
	input_A = h_coordinates;
	thrust::transform(input_A.begin(),input_A.end(),input_A.begin(),tf_vector());
	//h_coordinates = input_A;
	thrust::transform(d_coordinates.begin(),d_coordinates.end(),output_A.begin(),output_vector());
	//h_output_A = output_A;



	for(size_t epoch=0; epoch<1000; epoch++){ 
		//h_weights = d_weights;
		std::cout <<"Epoch:\t"<<epoch<<std::endl;;	
		//std::cout <<"Input coordinates"<<std::endl;;
		//for(const auto& coord:h_coordinates){
		//	std::cout << "(" << thrust::get<0>(coord) << ", " << thrust::get<1>(coord) << ")" << std::endl;;
		//}
		//std::cout <<"Output vector"<<std::endl;;
		//for(const auto& op: h_output_A){
		//	std::cout <<op<<std::endl;;
		//}
		//std::cout <<"Current weights"<<std::endl;;
		//for(const auto& iter : h_weights){
                //        std::cout <<"Weights :\t("<< thrust::get<0>(iter) <<","<< thrust::get<1>(iter) << ")"<<std::endl;;
                //}

		thrust::transform(input_A.begin(),input_A.end(),d_weights.begin(),result.begin(),DotProduct()); //phi(x) dot w(x)
		thrust::transform(result.begin(),result.end(),output_A.begin(),grad_desc_vec.begin(),2.0f*(_1 - _2)); // 2*((phi(x) dot (w(x)) - y) 	
		thrust::transform(result.begin(),result.end(),result.begin(),thrust::square<float>()); //(phi(x) dot w(x) - y) ^ 2
		thrust::transform(grad_desc_vec.begin(),grad_desc_vec.end(),input_A.begin(),d_weights_temp.begin(),[]__device__(float r,tup_pair input){return tup_pair (r*thrust::get<0>(input),r*thrust::get<1>(input));}); //(2*((phi(x) dot (w(x)) - y)))*phi(x)
																				 //h_weights = d_weights;

		float n_weights_0 = thrust::transform_reduce(d_weights_temp.begin(),d_weights_temp.end(),[]__device__(tup_pair input){return thrust::get<0>(input);},init,thrust::plus<float>());
		float n_weights_1 = thrust::transform_reduce(d_weights_temp.begin(),d_weights_temp.end(),[]__device__(tup_pair input){return thrust::get<1>(input);},init,thrust::plus<float>());
		n_weights = thrust::make_tuple(n_weights_0*eta/numPoints,n_weights_1*eta/numPoints);
		//std::cout<<"New weights_h :("<<thrust::get<0>(n_weights)<<","<<thrust::get<1>(n_weights)<<")"<<std::endl;;
		thrust::fill(d_weights_new.begin(),d_weights_new.end(),n_weights);
		thrust::transform(d_weights.begin(),d_weights.end(),d_weights_new.begin(),d_weights.begin(),tuple_subtract());

		tup_pair u_weights;
		u_weights = d_weights[0];
		std::cout<<"U_weights: ("<<thrust::get<0>(u_weights)<<","<<thrust::get<1>(u_weights)<<")"<<std::endl; //Print Updated weights after gradient descent
		//thrust::fill(d_weights.begin(),d_weights.end(),n_weights);
		
		//h_weights = d_weights;


		auto sum = thrust::reduce(result.begin(),result.end());
	
		float train_l = sum/numPoints;

	
		std::cout <<"TrainL = "<<train_l<<std::endl;;
	}



}
