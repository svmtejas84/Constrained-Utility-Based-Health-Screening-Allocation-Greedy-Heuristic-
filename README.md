# Constrained Utility-Based Health Screening Allocation (Greedy Heuristic)

This repository contains the C implementation and supporting documentation for a programming assignment in Design and Analysis of Algorithms (DAA). The project addresses a complex resource allocation problem in healthcare modeled as a variation of the NP-Hard Multi-Dimensional Knapsack Problem.

---

## Design and Analysis of Algorithms Assignment Report

### 1. Introduction

[cite_start]The efficient allocation of scarce resources in high-stakes environments such as healthcare presents a complex computational challenge that falls squarely within the domain of algorithm design[cite: 18]. [cite_start]This project addresses the problem of scheduling diagnostic screenings subject to multiple hard constraints: a fixed financial budget ($B_{\text{max}}$), limited immediate capacity (e.g., MRI machine time), constraints on future capacity, and a mandatory policy for equity (minimum service for low-criticality patients)[cite: 19]. [cite_start]The system's primary goal is to **maximize total utility** (patient benefit) while ensuring all constraints are strictly satisfied[cite: 20]. [cite_start]Since the problem is fundamentally a derivative of the **Multi-Dimensional Knapsack Problem (MDKP)**, which is NP-hard, the solution presented here employs a highly refined **Greedy Heuristic** integrated with state-tracking mechanisms borrowed from Dynamic Programming (DP) to achieve a near-optimal allocation efficiently[cite: 21]. [cite_start]The implementation is provided in **C**, a language chosen for its performance and suitability for algorithmic modeling[cite: 22]. [cite_start]The resulting allocation model prioritizes value-based care by selecting tests that yield the highest utility per unit of cost and resource demand, while dynamically adjusting cost metrics to reward consolidation and adherence to policy mandates[cite: 23].

---

### 2. Objective

[cite_start]The principal objective of this programming assignment is twofold: first, to design and implement a tractable solution to the Constrained Utility-Based Allocation problem [cite: 25][cite_start]; and second, to demonstrate the algorithmic reasoning behind the choices made, particularly the trade-off between optimality and efficiency[cite: 26].

The core computational task is to find a set of diagnostic tests $\mathcal{S} \subseteq T_{\text{set}}$ such that:

[cite_start]**Maximize:** $Total~Utility=\sum_{i\in\mathcal{S}}(W_{Crit,k}\times W_{Surv,k})$ [cite: 28]

**Subject to:**
* [cite_start]$C_{Marginal,i} \le B_{Max}$ [cite: 30, 31]
* [cite_start]$Feasibility(T_{i})$ holds for all $T_{i}\in\mathcal{S}$ [cite: 32]
* [cite_start]$Equity~Count \ge T_{Equity}$ [cite: 33]

[cite_start]The algorithm must effectively manage the state changes associated with resource consumption and marginal cost, demonstrating a robust technique for constraint propagation (via the Future Feasibility Check) that prevents local optimization from leading to future resource insolvency[cite: 34].

---

### 3. Functionality

[cite_start]The allocation is executed through a two-phase greedy process to ensure policy compliance before profit maximization[cite: 36].

#### Phase 1: Equity Enforcement

[cite_start]This initial phase is strictly mandatory and operates as a pre-processing constraint to satisfy the $T_{Equity}$ policy[cite: 38]. [cite_start]The algorithm first identifies the subset of tests that screen for low-criticality conditions (i.e., those with $TPS_{i}<TPS_{Cutoff}$)[cite: 39]. [cite_start]These low-utility items are then sorted using a secondary greedy criterion: **ascending immediate resource demand**[cite: 40]. [cite_start]This ensures the system selects the most resource-efficient low-criticality items first, thereby meeting the equity quota while minimizing consumption of bottleneck resources[cite: 41]. [cite_start]A test is scheduled only if it passes the immediate feasibility check and respects the budget[cite: 42]. [cite_start]This phase terminates exactly when the equity target count is met or when no further low-criticality tests can be scheduled due to resource/budget exhaustion[cite: 43].

#### Phase 2: Priority Triage (Utility Maximization)

[cite_start]Once the equity policy is satisfied, the algorithm transitions to maximizing overall utility using the remaining resources[cite: 45]. [cite_start]All unscheduled tests are sorted in **descending order** based on the primary greedy metric, the **Triage Priority Score (TPS)**[cite: 46]. [cite_start]The TPS calculation is dynamically updated here; if a shared resource setup cost ($R_j$) was incurred in Phase 1, subsequent tests will reflect a lower $C_{Marginal}$ cost, boosting their $TPS_{i}$ and rewarding resource consolidation[cite: 47].

[cite_start]The Triage Priority Score ($\text{TPS}_i$) is the primary greedy metric and described as follows[cite: 48]:
$$
[cite_start]TPS_{i}=\frac{W_{Crit,k}\times W_{Surv,k}}{C_{Marginal,i}+\sum_{j}D_{i,j}^{Time}} \text{ [cite: 49]}
$$
[cite_start]The numerator represents the utility gain, and the denominator represents the total resource cost[cite: 50]. [cite_start]Crucially, the cost term includes the **marginal monetary cost ($C_{Marginal,i}$)** which dynamically applies the large resource setup cost ($R_j$) only on the first use of that shared resource, rewarding resource consolidation[cite: 51]. [cite_start]Before scheduling the highest-TPS test, a **comprehensive feasibility check** is executed, simultaneously verifying both immediate capacity and the predicted impact on future treatment resources[cite: 52]. [cite_start]If either check fails, the test is permanently discarded for the current time slice[cite: 53].

---

### 4. Efficiency

[cite_start]The problem structure is equivalent to a variant of the **NP-Hard Multi-Dimensional Knapsack Problem (MDKP)**[cite: 55]. [cite_start]Due to the high complexity of MDKP and the sequential nature of resource state updates, a standard Dynamic Programming approach would require an excessive state space, making it computationally intractable for realistic scenarios[cite: 56].

[cite_start]The chosen **Greedy Heuristic** significantly reduces complexity[cite: 57]:

| Metric | Complexity | Reasoning |
| :--- | :--- | :--- |
| **Time Complexity** | $\mathbf{O}(N \log N)$ | [cite_start]The dominant operation is the sorting of the candidate test pool, which occurs in both Phase 1 and Phase 2[cite: 58]. [cite_start]If N is the number of tests, sorting takes $O(N \log N)$[cite: 58]. [cite_start]The allocation loop iterates at most N times, and the feasibility check inside takes $O(R)$ time, where R is the number of resources[cite: 58]. [cite_start]Since R is a constant, the total time complexity is $O(N \log N + N \cdot R)$, simplifying to $\mathbf{O}(N \log N)$[cite: 58]. |
| **Space Complexity** | $\mathbf{O}(N \cdot R + K)$ | [cite_start]The space usage is dominated by storing the primary data structures: the N test structs (each requiring $O(R)$ space for demand vectors), K condition structs and R resource structs[cite: 58]. [cite_start]The auxiliary arrays used for sorting the pools are copies of the test structs, maintaining the $O(N \cdot R)$ bound[cite: 58]. [cite_start]The algorithm avoids the exponential memory footprint associated with traditional DP tables, making it highly space-efficient and scalable to large test sets[cite: 58]. |

---

### 5. Code Quality

[cite_start]The C implementation is structured to ensure high modularity, readability, and correctness[cite: 60]. [cite_start]The entire system state is clearly defined using **structs** (`Condition`, `Test`, `Resource`), promoting strong typing and logical organization of related data[cite: 61]. [cite_start]Global arrays (`tests`, `resources`, `conditions`) manage the core data, reflecting the centralized nature of the allocation authority[cite: 62].

[cite_start]Key design decisions supporting quality include[cite: 63]:

* [cite_start]**Modularity** [cite: 64][cite_start]: Dedicated functions (`calculate_marginal_cost`, `calculate_tps`, `check_feasibility`, `schedule_test`) isolate complex mathematical and state-change logic[cite: 65]. [cite_start]This simplifies debugging and verification of the formulas[cite: 66].
* [cite_start]**State Management** [cite: 67][cite_start]: The `initialize_globals` function ensures a clean reset for testing and the `schedule_test` function is the single point of truth for all state transitions (updating $Cap_{\text{rem}}$, $Future\_Demand\_Commit$, and utilization flags)[cite: 68].
* [cite_start]**User Interface** [cite: 69][cite_start]: The inclusion of both a `setup_preset_data()` option and a `setup_user_input()` function controlled by the `main` loop provides a robust interface for both quick demonstration and detailed, custom scenario testing[cite: 70].

---

### 6. Debugging and Testing

[cite_start]The debugging strategy was centered on verifying the correct execution of the constraint propagation and the two-phase greedy priority[cite: 72]. [cite_start]Two distinct test modes were created for this purpose[cite: 73]:

* [cite_start]**Preset Data Testing (Baseline)** [cite: 74][cite_start]: The predefined scenario was constructed to specifically test the marginal cost and equity constraint[cite: 75]. [cite_start]The expected result was that Test T1 (resource-efficient, low-crit) would be scheduled first to satisfy the equity quota [cite: 76][cite_start], and the subsequent high-TPS tests would correctly calculate their marginal cost as $C_{explicit}$ (since the $R_{0}$ setup cost was already paid by T1)[cite: 77]. [cite_start]This ensured the core reward mechanism was functional[cite: 78].
* [cite_start]**Custom User Input Testing (Edge Cases)** [cite: 79][cite_start]: The user-input function was used to stress-test financial and capacity bottlenecks[cite: 80]:
    1.  [cite_start]**Budget Constraint** [cite: 81][cite_start]: Setting $B_{Max}$ to a value slightly less than the total cost of the top 3 $\text{TPS}$ items was used to verify that the algorithm terminates gracefully when the budget is exceeded, rather than over-scheduling[cite: 82].
    2.  [cite_start]**Future Feasibility Check** [cite: 83][cite_start]: Designing a high-TPS test with a very large $P_{pos}$ value (high probability of positive outcome) was used to check if the allocation was correctly blocked when the $D_{Future}$ commit overloaded the total capacity of Resource R1 (Treatment Beds), thus validating the look-ahead constraint[cite: 84].
    3.  [cite_start]**Equity Saturation** [cite: 85][cite_start]: A test was run to ensure that Phase 1 stops exactly at the `equity_target` and does not over-schedule low-utility items, preserving resources for Phase 2[cite: 86]. [cite_start]The print statements ([Equity] Scheduled Test...) provide clear, step-by-step visibility into the allocation choices during runtime[cite: 86].

---

