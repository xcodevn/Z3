/*++
Copyright (c) 2012 Microsoft Corporation

Module Name:

    smt_kernel.cpp

Abstract:

    New frontend for smt::context.
    
Author:

    Leonardo de Moura (leonardo) 2012-02-09.

Revision History:

--*/
#include"smt_kernel.h"
#include"smt_context.h" 
#include"ast_smt2_pp.h"
#include"params2front_end_params.h"

namespace smt {

    struct kernel::imp {
        smt::context m_kernel;
        params_ref   m_params;
        
        imp(ast_manager & m, front_end_params & fp, params_ref const & p):
            m_kernel(m, fp, p),
            m_params(p) {
        }

        front_end_params & fparams() {
            return m_kernel.get_fparams();
        }

        params_ref const & params() {
            return m_params;
        }
     
        ast_manager & m() const {
            return m_kernel.get_manager();
        }

        bool set_logic(symbol logic) {
            return m_kernel.set_logic(logic);
        }
        
        void set_progress_callback(progress_callback * callback) {
            return m_kernel.set_progress_callback(callback);
        }
        
        void assert_expr(expr * e) {
            TRACE("smt_kernel", tout << "assert:\n" << mk_ismt2_pp(e, m()) << "\n";);
            m_kernel.assert_expr(e);
        }
        
        void assert_expr(expr * e, proof * pr) {
            m_kernel.assert_expr(e, pr);
        }

        unsigned size() const {
            return m_kernel.get_num_asserted_formulas();
        }
        
        expr * const * get_formulas() const {
            return m_kernel.get_asserted_formulas();
        }

        expr * get_quantifier_instance() {
            return m_kernel.quantifier_instance.get();
        }
        
        bool reduce() {
            return m_kernel.reduce_assertions();
        }
        
        void push() {
            TRACE("smt_kernel", tout << "push()\n";);
            m_kernel.push();
        }

        void pop(unsigned num_scopes) {
            TRACE("smt_kernel", tout << "pop()\n";);
            m_kernel.pop(num_scopes);
        }
        
        unsigned get_scope_level() const {
            return m_kernel.get_scope_level();
        }

        lbool setup_and_check() {
            return m_kernel.setup_and_check();
        }

        bool inconsistent() {
            return m_kernel.inconsistent();
        }
        
        lbool check(unsigned num_assumptions, expr * const * assumptions) {
            return m_kernel.check(num_assumptions, assumptions);
        }
        
        void get_model(model_ref & m) const {
            m_kernel.get_model(m);
        }

        proof * get_proof() {
            return m_kernel.get_proof();
        }

        unsigned get_unsat_core_size() const {
            return m_kernel.get_unsat_core_size();
        }
        
        expr * get_unsat_core_expr(unsigned idx) const {
            return m_kernel.get_unsat_core_expr(idx);
        }
        
        failure last_failure() const {
            return m_kernel.get_last_search_failure();
        }
        
        std::string last_failure_as_string() const {
            return m_kernel.last_failure_as_string();
        }

        void get_assignments(expr_ref_vector & result) {
            m_kernel.get_assignments(result);
        }
        
        void get_relevant_labels(expr * cnstr, buffer<symbol> & result) {
            m_kernel.get_relevant_labels(cnstr, result);
        }
        
        void get_relevant_labeled_literals(bool at_lbls, expr_ref_vector & result) {
            m_kernel.get_relevant_labeled_literals(at_lbls, result);
        }

        void get_relevant_literals(expr_ref_vector & result) {
            m_kernel.get_relevant_literals(result);
        }
        
        void get_guessed_literals(expr_ref_vector & result) {
            m_kernel.get_guessed_literals(result);
        }

        void display(std::ostream & out) const {
            // m_kernel.display(out); <<< for external users it is just junk
            // TODO: it will be replaced with assertion_stack.display
            unsigned num = m_kernel.get_num_asserted_formulas();
            expr * const * fms = m_kernel.get_asserted_formulas();
            out << "(kernel";
            for (unsigned i = 0; i < num; i++) {
                out << "\n  " << mk_ismt2_pp(fms[i], m(), 2);
            }
            out << ")";
        }
        
        void collect_statistics(::statistics & st) const {
            m_kernel.collect_statistics(st);
        }
        
        void reset_statistics() {
        }

        void display_statistics(std::ostream & out) const {
            m_kernel.display_statistics(out);
        }
        
        void display_istatistics(std::ostream & out) const {
            m_kernel.display_istatistics(out);
        }

        void set_cancel(bool f) {
            m_kernel.set_cancel_flag(f);
        }
        
        bool canceled() {
            return m_kernel.get_cancel_flag();
        }

        void updt_params(params_ref const & p) {
            params2front_end_params(p, fparams());
        }
    };

    kernel::kernel(ast_manager & m, front_end_params & fp, params_ref const & p) {
        m_imp = alloc(imp, m, fp, p);
    }

    kernel::~kernel() {
        dealloc(m_imp);
    }

    ast_manager & kernel::m() const {
        return m_imp->m();
    }

    bool kernel::set_logic(symbol logic) {
        return m_imp->set_logic(logic);
    }

    void kernel::set_progress_callback(progress_callback * callback) {
        m_imp->set_progress_callback(callback);
    }

    void kernel::assert_expr(expr * e) {
        m_imp->assert_expr(e);
    }

    void kernel::assert_expr(expr * e, proof * pr) {
        m_imp->assert_expr(e, pr);
    }

    unsigned kernel::size() const {
        return m_imp->size();
    }
    
    expr * const * kernel::get_formulas() const {
        return m_imp->get_formulas();
    }

    bool kernel::reduce() {
        return m_imp->reduce();
    }

    void kernel::push() {
        m_imp->push();
    }

    void kernel::pop(unsigned num_scopes) {
        m_imp->pop(num_scopes);
    }

    unsigned kernel::get_scope_level() const {
        return m_imp->get_scope_level();
    }

    void kernel::reset() {
        ast_manager & _m       = m();
        front_end_params & fps = m_imp->fparams();
        params_ref ps          = m_imp->params();
        #pragma omp critical (smt_kernel)
        {
            dealloc(m_imp);
            m_imp = alloc(imp, _m, fps, ps);
        }
    }

    bool kernel::inconsistent() {
        return m_imp->inconsistent();
    }

    lbool kernel::setup_and_check() {
        set_cancel(false);
        return m_imp->setup_and_check();
    }

    lbool kernel::check(unsigned num_assumptions, expr * const * assumptions) {
        set_cancel(false);
        lbool r = m_imp->check(num_assumptions, assumptions);
        TRACE("smt_kernel", tout << "check result: " << r << "\n";);
        return r;
    }

    void kernel::get_model(model_ref & m) const {
        m_imp->get_model(m);
    }

    proof * kernel::get_proof() {
        return m_imp->get_proof();
    }

    unsigned kernel::get_unsat_core_size() const {
        return m_imp->get_unsat_core_size();
    }
        
    expr * kernel::get_unsat_core_expr(unsigned idx) const {
        return m_imp->get_unsat_core_expr(idx);
    }

    failure kernel::last_failure() const {
        return m_imp->last_failure();
    }

    std::string kernel::last_failure_as_string() const {
        return m_imp->last_failure_as_string();
    }

    void kernel::get_assignments(expr_ref_vector & result) {
        m_imp->get_assignments(result);
    }
        
    void kernel::get_relevant_labels(expr * cnstr, buffer<symbol> & result) {
        m_imp->get_relevant_labels(cnstr, result);
    }
    
    void kernel::get_relevant_labeled_literals(bool at_lbls, expr_ref_vector & result) {
        m_imp->get_relevant_labeled_literals(at_lbls, result);
    }

    void kernel::get_relevant_literals(expr_ref_vector & result) {
        m_imp->get_relevant_literals(result);
    }

    void kernel::get_guessed_literals(expr_ref_vector & result) {
        m_imp->get_guessed_literals(result);
    }

    void kernel::display(std::ostream & out) const {
        m_imp->display(out);
    }

    void kernel::collect_statistics(::statistics & st) const {
        m_imp->collect_statistics(st);
    }
        
    void kernel::reset_statistics() {
        m_imp->reset_statistics();
    }

    void kernel::display_statistics(std::ostream & out) const {
        m_imp->display_statistics(out);
    }

    void kernel::display_istatistics(std::ostream & out) const {
        m_imp->display_istatistics(out);
    }

    void kernel::set_cancel(bool f) {
        #pragma omp critical (smt_kernel)
        {
            if (m_imp)
                m_imp->set_cancel(f);
        }
    }

    bool kernel::canceled() const {
        return m_imp->canceled();
    }

    void kernel::updt_params(params_ref const & p) {
        return m_imp->updt_params(p);
    }

    void kernel::collect_param_descrs(param_descrs & d) {
        solver_front_end_params_descrs(d);
    }

    context & kernel::get_context() {
        return m_imp->m_kernel;
    }

    expr * kernel::get_quantifier_instance() {
        return m_imp->get_quantifier_instance();
    }

};
