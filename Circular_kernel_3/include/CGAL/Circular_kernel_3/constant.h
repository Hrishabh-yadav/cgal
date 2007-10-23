#ifndef CGAL_SPHERICAL_KERNEL_CONSTANT_H
#define CGAL_SPHERICAL_KERNEL_CONSTANT_H

namespace CGAL{
  typedef float HQ_NT;//type to represent the index of one hquadrant
  enum Fct_type{TAN, COT, FIXED, TAG_M2};
  enum Circle_type{NORMAL,THREADED,POLAR,BIPOLAR};
  
  inline Fct_type auto_ftype(const HQ_NT& hquad){
  if (hquad >7 || hquad<2 || (hquad>3 && hquad<6))
    return TAN;
  return COT;
  };    

  template<class SK>
  typename CGAL::HQ_NT hquadrant(const typename SK::Circular_arc_point_3& R)
  {
    int x=CGAL::sign(R.x());
    int y=CGAL::sign(R.y());
    if (y>0){
      if (x>0)  switch (CGAL::sign(R.x()-R.y())){case -1: return 2; break;  case 0: return 1.5; break; case 1: return 1; break; }; 
      if (x<0) switch (CGAL::opposite(CGAL::sign(R.x()+R.y()))){case -1: return 3; break;  case 0: return 3.5; break; case 1: return 4; break; };
      return 2.5;//OPTI : we have more information here by x_y
    }
    else{
      if (y<0){
        if (x>0) switch (CGAL::sign(R.x()+R.y())){case -1: return 7; break;  case 0: return 7.5; break; case 1: return 8; break; };
        if (x<0) switch (CGAL::opposite(CGAL::sign(R.x()-R.y()))){case -1: return 6; break; case 0: return 5.5; break; case 1: return 5; break; };
        return 6.5;//OPTI : we have more information here by x_y
      }
      else{
        if (x>0)  return 0.5;
        if (x<0) return 4.5;
        return 0;
      }
    }
  }

  
  struct Inter_alg_info{
    CGAL::HQ_NT qF;
    CGAL::HQ_NT qS;
    int F_index;
    int S_index;
    bool is_polar;
    Inter_alg_info()
      :qF(-1),qS(-1),F_index(-1),S_index(-1),is_polar(false){};
    void print() const{
      std::cout << "(" << qF <<","<< qS <<") ("<< F_index <<","<< S_index <<") " << std::endl;
    }
  };

  bool operator==(const Inter_alg_info& IA1,const Inter_alg_info& IA2){
    return (IA1.qF==IA2.qF) && (IA1.qS==IA2.qS) && (IA1.F_index==IA2.F_index) && (IA1.S_index==IA2.S_index);
  }  
  
  
  template <class T>
  inline static const T& get_point(const T& p){return p;};
  
  template <class T>
  inline static const T& get_point(const std::pair<T,unsigned>& p){return p.first;}  
  
  template<class SK,class pt_container>
  static void init_indices(CGAL::Inter_alg_info& IA,const pt_container& Ipts,bool is_tangency){
    
    if (is_tangency){ //OPTI here adapt to have square free polynomial for tangency or do "if Root_of.poly().size()<3"
      IA.qF=hquadrant<SK>(get_point(Ipts[0]));
      IA.F_index=0;
      IA.S_index=0;
      IA.qS=IA.qF;      
      return;
    }
    
    CGAL::HQ_NT q0=hquadrant<SK>(get_point(Ipts[0]));
    CGAL::HQ_NT q1=hquadrant<SK>(get_point(Ipts[1]));

    
    CGAL::HQ_NT res=q0-q1;
    if (res == 0){//same quadrant
      res=CGAL::sign(get_point(Ipts[0]).y()*get_point(Ipts[1]).x()-get_point(Ipts[1]).y()*get_point(Ipts[0]).x());
      if (res==0){//same f(theta) value
        res=typename SK::Compare_z_3() (get_point(Ipts[1]),get_point(Ipts[0]));
        CGAL_precondition(res!=0);//DEBUG
      }
    }
    if (res <0){
      IA.qF=(q0==0)?(10):(q0);//hande 2 polar by same pole
      IA.qS=q1;
      IA.F_index=0;
      IA.S_index=1;
    }
    else{
      IA.qS=q0;
      IA.qF=(q1==0)?(10):(q1);//hande  2 polar by same pole
      IA.F_index=1;
      IA.S_index=0;
    }
  };
  
  template<class G>
  inline void exchange(G& v1,G& v2)
  {
    G tmp=v2;
    v2=v1;
    v1=tmp;
  }  
  
  //Never more than Pi between two intersection points
  template<class pt_container>
  static void set_inter_pt_conv(CGAL::Inter_alg_info& IA,const pt_container& Ipts){
    if (IA.qS-IA.qF>4){
      exchange(IA.F_index,IA.S_index);
      exchange(IA.qF,IA.qS);
    }
    else{
      if (!(IA.qS-IA.qF<4)){//cad =4  |  conflict who is the first?
        int s=CGAL::sign(get_point(Ipts[IA.F_index]).y()*get_point(Ipts[IA.S_index]).x()-get_point(Ipts[IA.S_index]).y()*get_point(Ipts[IA.F_index]).x());
        if (s>0){
          exchange(IA.F_index,IA.S_index);
          exchange(IA.qF,IA.qS);
        }
      }
    }
  }

  template<class SK,class pt_container>
  static void set_IA(CGAL::Inter_alg_info& IA,const pt_container& Ipts,bool is_tangency){
    init_indices<SK>(IA,Ipts,is_tangency);
    if (!is_tangency) set_inter_pt_conv(IA,Ipts);
  }  
  
  
  #warning introduce this in a better manner
  template<class FT,class Point_3>
  inline FT compute_a(const Point_3& c,const FT& R2,const FT& squared_radius){
    return c.x() * c.x() + c.y() * c.y()+ c.z() * c.z() + R2 - squared_radius;
  };
  
  template<class FT>
  FT circle_center_coefficent(const FT& x,const FT& y,const FT& z,const FT& r2,const FT& R2){
    return ((FT)(0.5) +  (R2 - r2)/(FT)(2* (x*x +y*y +z*z))) ;
  }
  
  template<class SK, class circle_on_sphere>
  CGAL::Circle_type classify_one_circle(const circle_on_sphere& C){
    if (C.supporting_sphere().center().z()==0){
      typename SK::Point_3 Pt=C.center();
      if (Pt.z()==0 && Pt.y()==0 && Pt.x()==0)
      return CGAL::BIPOLAR;
    }
    std::vector<CGAL::Object> cont;
    typename SK::Plane_3 Pl=SK().construct_plane_3_object()(typename SK::Algebraic_kernel::Polynomial_1_3(0,1,0,0));
    typename SK::Intersect_3()(C.reference_sphere(),C.supporting_sphere(),Pl,std::back_inserter(cont));
    
    switch (cont.size()){
      case 0: 
        return CGAL::NORMAL;
      case 2:{ 
        std::pair<typename SK::Circular_arc_point_3,unsigned> p1,p2;
        CGAL::assign(p1,cont[0]);CGAL::assign(p2,cont[1]);
        CGAL::Sign s1=CGAL::sign(p1.first.x());
        CGAL::Sign s2=CGAL::sign(p2.first.x());
        if (s1==CGAL::opposite(s2))
          return CGAL::THREADED;
        else
          if (s1!=s2) return CGAL::POLAR;
        }
        break;
      
      case 1:
        if (CGAL::abs(C.extremal_point_z())==C.reference_sphere().radius()) return CGAL::POLAR;
    }
    return CGAL::NORMAL;
  }  
  
  
}

#endif
