import os
import re
import subprocess
from typing import Optional, Any


import pddl
import pddl.logic
import pddl.logic.functions
from pddl.core import Domain as PDDLDomain

from _wlplan.planning import (
    Atom,
    ComparatorType,
    ConstantExpression,
    Domain,
    Fluent,
    FluentExpression,
    FormulaExpression,
    Function,
    NumericCondition,
    NumericExpression,
    OperatorType,
    Predicate,
    ActionSchema,
    Action,
    Problem,
    GroundedProblem
)

_CUR_DIR = os.path.dirname(os.path.abspath(__file__))

__all__ = ["parse_domain", "parse_problem"]

_PDDL_TO_WLPLAN_BINARY_OPS = {
    pddl.logic.functions.Plus: OperatorType.Plus,
    pddl.logic.functions.Minus: OperatorType.Minus,
    pddl.logic.functions.Times: OperatorType.Multiply,
    pddl.logic.functions.Divide: OperatorType.Divide,
}


def _get_predicates(pddl_domain: PDDLDomain, keep_statics: bool) -> dict[str, Predicate]:
    predicates = {}
    if keep_statics:
        for predicate in pddl_domain.predicates:
            name = predicate.name
            arity = predicate.arity
            predicates[name] = Predicate(name=name, arity=arity)
    else:
        for schema in pddl_domain.actions:
            if isinstance(schema.effect, pddl.logic.base.And):
                effects = schema.effect._operands
            else:
                effects = [schema.effect]
            for effect in effects:
                if isinstance(effect, pddl.logic.predicates.Predicate):
                    predicate = effect
                elif isinstance(effect, pddl.logic.base.Not):
                    predicate = effect._arg
                else:
                    continue
                name = predicate.name
                arity = predicate.arity
                predicate = Predicate(name=name, arity=arity)
                if name not in predicates:
                    predicates[name] = predicate
                else:
                    assert predicates[name] == predicate
    return predicates


def _get_functions(pddl_domain: PDDLDomain) -> dict[str, Function]:
    functions = {}
    for function in pddl_domain.functions:
        name = function.name
        arity = function.arity
        functions[name] = Function(name=name, arity=arity)
    return functions

def _get_action_schemas(pddl_domain: PDDLDomain) -> dict[str, Any]:
    action_schemas = {}
    for action_schema in pddl_domain.actions:
        name = action_schema.name
        arity = len(action_schema.parameters)
        action_schemas[action_schema.name] = ActionSchema(name=name, arity=arity)

    return action_schemas

def _create_sas_translation(domain_pddl: str, problem_pddl: str) -> str:
    script = f"{_CUR_DIR}/translate/translate.py"
    sas_file = str(hash(domain_pddl)) + "_" + str(hash(problem_pddl))
    sas_file = sas_file.replace("-", "0") + ".sas"
    cmd = [script, "--sas-file", sas_file, domain_pddl, problem_pddl]
    subprocess.check_output(cmd, universal_newlines=True)
    with open(sas_file, "r") as f:
        content = f.read()
    os.remove(sas_file)
    return content

def _get_variables(sas_content: str) -> tuple[list[str], list[int]]:
    variable_blocks: list[str] = re.findall(r"^begin_variable(?s:(.+?))end_variable$", 
                                            sas_content, re.MULTILINE | re.DOTALL)

    variable_names = []
    variable_sizes = []
    for block in variable_blocks:
        block = [x for x in block.split("\n") if len(x) > 0]
        variable_names.append(block[0])
        variable_sizes.append(int(block[2]))

    return variable_names, variable_sizes

def _get_goals(sas_content: str) -> list[tuple[int, int]]:
    goals_block = re.search(r"^begin_goal(?s:(.+))end_goal$",
                           sas_content, re.MULTILINE | re.DOTALL)
    
    if goals_block is not None:
        goals_block = goals_block.group().split("\n")

        goals = []
        for goal in goals_block[2:-1]:
            goals.append(tuple([int(x) for x in goal.split()]))
        
        return goals

    return []

def _get_actions(sas_content: str, domain: Domain) -> list[Action]:
    action_name_to_schema = {a.name: a for a in domain.action_schemas}

    operators = re.findall(r"^begin_operator(?s:(.+?))end_operator$",
                           sas_content, re.MULTILINE | re.DOTALL)
    
    actions = []
    for op in operators:
        index = 0
        op: list[str] = [x for x in op.split("\n") if len(x) > 0]

        op_name = op[index].split()[0]
        action_schema = action_name_to_schema[op_name]
        index += 1

        preconditions_size = int(op[index])
        preconditions = set()
        index += 1
        for _ in range(preconditions_size):
            precond = tuple(int(x) for x in op[index].split())
            preconditions.add(precond)
            index += 1
        
        effects_size = int(op[index])
        effects = set()
        index += 1
        for _ in range(effects_size):
            effect = [int(x) for x in op[index].split()[1:]]

            if effect[1] > -1:
                preconditions.add(tuple(effect[:2]))
            
            effects.add((effect[0], effect[2]))

            index += 1

        action = Action(
            action_schema=action_schema,
            preconditions=preconditions,
            effects=effects
        )

        actions.append(action)

    return actions


def _convert_pddl_to_wlplan_expression(
    pddl_expression, fluent_to_id: dict[str, int]
) -> NumericExpression:
    if isinstance(pddl_expression, pddl.logic.functions.NumericFunction):
        toks = str(pddl_expression).replace(")", "").replace("(", "").split()
        fluent_name = toks[0] + "(" + ", ".join(toks[1:]) + ")"
        assert fluent_name in fluent_to_id
        expression = FluentExpression(fluent_to_id[fluent_name], fluent_name)
    elif isinstance(pddl_expression, pddl.logic.functions.NumericValue):
        value = float(pddl_expression.value)
        expression = ConstantExpression(value)
    else:
        assert isinstance(pddl_expression, tuple(_PDDL_TO_WLPLAN_BINARY_OPS.keys()))
        a = _convert_pddl_to_wlplan_expression(pddl_expression._operands[0], fluent_to_id)
        b = _convert_pddl_to_wlplan_expression(pddl_expression._operands[1], fluent_to_id)
        op_type = _PDDL_TO_WLPLAN_BINARY_OPS[type(pddl_expression)]
        expression = FormulaExpression(op_type, a, b)
    return expression


def parse_domain(
    domain_path: str,
    domain_name: Optional[str] = None,
    keep_statics: bool = True,
) -> Domain:
    """Parses a domain file and returns a Domain object.

    Args:
        domain_path (str): The path to the domain file.
        domain_name (str, optional): The name of the domain. If not provided, it will be extracted from the file. Defaults to None.
        keep_statics (bool, optional): Whether to keep static predicates in the domain, computed by taking the union of action effects. Defaults to True.
    """

    if not os.path.exists(domain_path):
        raise FileNotFoundError(f"Domain file {domain_path} does not exist.")

    with open(domain_path, "r") as f:
        domain_content = f.read()

    # Read domain name from file if not provided
    if domain_name is None:
        domain_name = domain_content.split("(domain ")[1]
        domain_name = domain_name.split(")")[0]

    # Parse domain with the pddl package
    pddl_domain = pddl.parse_domain(domain_path)

    # Get predicates
    predicates = _get_predicates(pddl_domain, keep_statics)
    predicates = sorted(list(predicates.values()), key=lambda x: repr(x))

    # Get functions
    functions = _get_functions(pddl_domain)
    functions = sorted(list(functions.values()), key=lambda x: repr(x))

    # Get action schemas
    action_schemas = _get_action_schemas(pddl_domain)
    action_schemas = sorted(list(action_schemas.values()), key=lambda x: repr(x))

    # Get constant objects (ignores types)
    constant_objects = sorted(list(str(o) for o in pddl_domain.constants))

    domain = Domain(
        name=domain_name,
        predicates=predicates,
        functions=functions,
        constant_objects=constant_objects,
        action_schemas=action_schemas
    )
    return domain


def parse_problem(domain_path: str, problem_path: str, keep_statics: bool = True) -> Problem:
    """Parses a problem file and returns a Problem object.

    Args:
        domain_path (str): The path to the domain file.
        problem_path (str): The path to the problem file.
        keep_statics (bool, optional): Whether to keep static predicates in the parsed domain. Defaults to True.
    """

    if not os.path.exists(problem_path):
        raise FileNotFoundError(f"Problem file {problem_path} does not exist.")

    # Use the pddl package to help with parsing
    pddl_domain = pddl.parse_domain(domain_path)
    pddl_problem = pddl.parse_problem(problem_path)

    # Get domain information
    name_to_predicate = _get_predicates(pddl_domain, keep_statics)
    name_to_function = _get_functions(pddl_domain)

    # Get problem information
    objects = sorted(o.name for o in pddl_problem.objects)
    statics = []  # TODO: also requires checking keep_statics flag
    fluents = []
    fluent_values = []
    fluent_to_id = {}

    for formula in sorted(pddl_problem.init, key=lambda x: repr(x)):
        if not isinstance(formula, pddl.logic.functions.EqualTo):
            continue
        variable = formula._operands[0]
        value = float(formula._operands[1].value)
        function = name_to_function[variable.name]
        fluent_terms = [o.name for o in variable.terms]
        fluent_values.append(value)
        fluent = Fluent(function=function, objects=fluent_terms)
        fluent_to_id[str(fluent)] = len(fluents)
        fluents.append(fluent)

    # Get goal information
    wlplan_goals = {"positive": [], "negative": [], "numeric": []}

    goals = pddl_problem.goal
    if isinstance(goals, pddl.logic.base.And):  # multiple goals
        goals = goals._operands
    else:  # single goal
        goals = [goals]

    def handle_propositional_goal(goal):
        if isinstance(goal, pddl.logic.predicates.Predicate):
            a = goal
            goal_type = "positive"
        elif isinstance(goal, pddl.logic.base.Not):
            a = goal._arg
            goal_type = "negative"
        wlplan_atom = Atom(predicate=name_to_predicate[a.name], objects=[o.name for o in a.terms])
        wlplan_goals[goal_type].append(wlplan_atom)

    def handle_numeric_goal(goal):
        # Convert to a normal form of [expression \unrhd 0]
        lhs = goal._operands[0]
        rhs = goal._operands[1]
        if isinstance(goal, pddl.logic.functions.EqualTo):
            # lhs  = rhs ---> lhs - rhs =  0
            comparator_type = ComparatorType.Equal
            expression = pddl.logic.functions.Minus(lhs, rhs)
        elif isinstance(goal, pddl.logic.functions.LesserThan):
            # lhs <  rhs ---> rhs - lhs >  0
            comparator_type = ComparatorType.GreaterThan
            expression = pddl.logic.functions.Minus(rhs, lhs)
        elif isinstance(goal, pddl.logic.functions.LesserEqualThan):
            # lhs <= rhs ---> rhs - lhs >= 0
            comparator_type = ComparatorType.GreaterEqualThan
            expression = pddl.logic.functions.Minus(rhs, lhs)
        elif isinstance(goal, pddl.logic.functions.GreaterThan):
            # lhs >  rhs ---> lhs - rhs >  0
            comparator_type = ComparatorType.GreaterThan
            expression = pddl.logic.functions.Minus(lhs, rhs)
        elif isinstance(goal, pddl.logic.functions.GreaterEqualThan):
            # lhs >= rhs ---> lhs - rhs >= 0
            comparator_type = ComparatorType.GreaterEqualThan
            expression = pddl.logic.functions.Minus(lhs, rhs)
        expression = _convert_pddl_to_wlplan_expression(expression, fluent_to_id)
        numeric_goal = NumericCondition(comparator_type=comparator_type, expression=expression)
        wlplan_goals["numeric"].append(numeric_goal)

    for goal in goals:
        # propositional goals
        if isinstance(goal, (pddl.logic.predicates.Predicate, pddl.logic.base.Not)):
            handle_propositional_goal(goal)
        elif isinstance(
            goal,
            (
                pddl.logic.functions.EqualTo,
                pddl.logic.functions.LesserThan,
                pddl.logic.functions.LesserEqualThan,
                pddl.logic.functions.GreaterThan,
                pddl.logic.functions.GreaterEqualThan,
            ),
        ):
            handle_numeric_goal(goal)
        else:
            raise ValueError(f"Unknown goal {goal} with type {type(goal)}")

    problem = Problem(
        domain=parse_domain(domain_path),
        objects=objects,
        statics=statics,
        fluents=fluents,
        fluent_values=fluent_values,
        positive_goals=wlplan_goals["positive"],
        negative_goals=wlplan_goals["negative"],
        numeric_goals=wlplan_goals["numeric"],
    )
    return problem

def parse_grounded_problem(domain_path: str, problem_path: str):
    """Parses a problem file and returns a GroundedProblem object.

    Args:
        domain_path (str): The path to the domain file.
        problem_path (str): The path to the problem file.
    """

    if not os.path.exists(problem_path):
        raise FileNotFoundError(f"Problem file {problem_path} does not exist.")
    
    # domain
    domain = parse_domain(domain_path)

    # Translate to SAS+
    sas_content = _create_sas_translation(domain_path, problem_path)

    # variables
    variable_names, variable_sizes  =_get_variables(sas_content)
    
    # goals
    goals = _get_goals(sas_content)

    # actions
    actions = _get_actions(sas_content, domain)

    return GroundedProblem(
        domain=domain,
        variable_names=variable_names,
        variable_domain_sizes=variable_sizes,
        goals=goals,
        actions=actions
    )


if __name__ == "__main__":
    # Quick test
    import os

    CUR_DIR = os.path.dirname(os.path.abspath(__file__))

    benchmark_dir = f"{CUR_DIR}/../tests/cost_part"
    domain_path = f"{benchmark_dir}/benchmarks/blocks/domain.pddl"
    problem_path = f"{benchmark_dir}/benchmarks/blocks/probBLOCKS-4-0.pddl"
    if not os.path.exists(domain_path):
        import zipfile

        zip_file_path = f"{benchmark_dir}/benchmarks.zip"
        extract_dir = f"{benchmark_dir}"
        with zipfile.ZipFile(zip_file_path, "r") as zip_ref:
            zip_ref.extractall(extract_dir)
    keep_statics = False
    grounded_problem = parse_grounded_problem(domain_path, problem_path)
    grounded_problem.dump()
