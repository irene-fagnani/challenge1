#include <functional>
#include <vector>
#include <cmath>
#include "helper.hpp"

#ifndef METHOD_HPP
#define METHOD_HPP
    
     // check convergence
   bool check_convergence(std::vector<value_type> & xold,std::vector<value_type> & xnew, unsigned int k,std::vector<value_type> & grad,input _input){
        if(k>_input.it){
          std::cout<<"\nLimit of the max number of iterations: no convergence is reached.\n"<<std::endl;
        }else if(euclidean_norm(xold-xnew)<_input.eps_s){
          std::cout<<"\nThe convergence is reached:  ∥ x_k+1 - x_k ∥ < ε_s\n"<<std::endl;
        }else if(euclidean_norm(grad)<_input.eps_r){
          std::cout<<"\nThe convergence is reached: ∥ ∇ f(x_k) ∥ < ε_r\n"<<std::endl;
        }
        return k>_input.it or euclidean_norm(xold-xnew)<_input.eps_s or euclidean_norm(grad)<_input.eps_r;
    }
    
    template<Diff D>
    std::vector<value_type> compute_gradient(std::vector<value_type> & x,input _input){
      std::vector<value_type> grad(x.size(),0);
      if constexpr(D==Diff::finite_diff){
         for(size_t i=0;i<x.size();++i){
        std::vector<value_type> v1(x),v2(x);
        v1[i]+=_input.h;
        v2[i]-=_input.h;
        grad[i]=((_input.f(v1)-_input.f(v2))/(2.*_input.h));
        }
      }
      if constexpr(D==Diff::user_grad){
        grad=_input.df(x);
      }
      return grad;
    }

    template<Diff D,Alpha A,Mode M>
     std::vector<value_type> solve(input _input){
     print_struct<D,A,M>(_input);
     bool flag=false;
     value_type ak=_input.a0;
     unsigned int k=0;

     std::vector<value_type> grad((_input.start).size(),0);
     std::vector<value_type> grad_y((_input.start).size(),0);
     std::vector<value_type> xold((_input.start.size()),0);
     std::vector<value_type> xnew(_input.start);
     std::vector<value_type> xold1(_input.start);
     if constexpr(M==Mode::gradient){
      xold=_input.start;
     }

     if constexpr(M==Mode::Heavy_Ball){
      xold=_input.start-_input.a0*compute_gradient<D>(_input.start,_input);
     }
      
      if constexpr(M==Mode::Nesterov){
        xold=_input.start-_input.a0*compute_gradient<D>(_input.start,_input);
      }

     if(A==Alpha::Armijo && (M==Mode::Heavy_Ball || M==Mode::Nesterov)){
      flag=true;
      std::cerr<<"Cannot use Armijo computation for the update of alpha_k and Heavy-Ball method for solving the minimization problem.\n"<<std::endl;
      exit(1);
     }

     while(!flag){

    // update number of iterations
    ++k;

    //gradient evaluation
    grad=compute_gradient<D>(xold,_input);
    // xnew update
    if constexpr(M==Mode::Heavy_Ball){
      xnew=xold-ak*grad+_input.eta*(xold-xold1);
    }
    if constexpr(M==Mode::gradient){
      xnew=xold-ak*grad;
    }
    if constexpr(M==Mode::Nesterov){
        // gradient y evaluation
        std::vector<value_type> y=(xold+_input.eta*(xold-xold1));
        // xnew computation
        xnew=xold+_input.eta*(xold-xold1)-ak*compute_gradient<D>(y,_input);
    }
      // check convergence
      flag=check_convergence(xold,xnew,k,grad,_input);

      // alpha computation
      if constexpr(A==Alpha::Armijo){
        ak=_input.a0;
        while(_input.f(xnew)-_input.f(xnew-ak*compute_gradient<D>(xnew,_input)) < _input.sigma * ak * euclidean_norm(compute_gradient<D>(xnew,_input)) *euclidean_norm(compute_gradient<D>(xnew,_input))){         
        ak/=2;
      }
      }
      if constexpr(A==Alpha::exponential_decay){
        ak=_input.a0*exp(-_input.mu*static_cast<value_type>(k));
      }

      if constexpr(A==Alpha::inverse_decay){
        ak=_input.a0/(1+(_input.mu*static_cast<value_type>(k)));
      }

      // update
      xold1=xold;
      xold=xnew;

      }
     std::cout<<"Number of interations: "<<k<<"\n"<<std::endl;
    return xnew;
  }
  
#endif
