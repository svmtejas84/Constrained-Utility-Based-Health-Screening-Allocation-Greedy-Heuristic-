#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_TESTS 100
#define MAX_RESOURCES 10
#define MAX_CONDITIONS 50

#define T_EQUITY 0.15 
#define TPS_CUTOFF 50.0 

#define TIME_HORIZON 1 

typedef struct {
    int condition_id;
    double W_Crit;
    double W_Surv;
} Condition;

typedef struct {
    int test_id;
    int condition_k;
    double C_Explicit;
    double P_Pos;
    double TPS;
    
    double D_Immediate[MAX_RESOURCES];
    double D_Future_Calc[MAX_RESOURCES];
    double Marginal_Cost;
} Test;

typedef struct {
    double Cap_Total;
    double Cap_Rem;
    bool Utilized;
    double R_SetupCost;
    double Future_Demand_Commit;
} Resource;

Test tests[MAX_TESTS];
Resource resources[MAX_RESOURCES];
Condition conditions[MAX_CONDITIONS];
int num_tests = 0;
int num_resources = 0;
int num_conditions = 0;
double B_Max = 0.0;

bool is_scheduled[MAX_TESTS] = {false};
double total_cost = 0.0;
double total_utility = 0.0;
int equity_count = 0;

void initialize_globals() {
    total_cost = 0.0;
    total_utility = 0.0;
    equity_count = 0;
    for (int i = 0; i < MAX_TESTS; i++) {
        is_scheduled[i] = false;
    }
}

void calculate_marginal_cost(Test *test, const Resource *resources) {
    test->Marginal_Cost = test->C_Explicit;
    for (int j = 0; j < num_resources; j++) {
        if (resources[j].R_SetupCost > 0.0 && !resources[j].Utilized) {
            test->Marginal_Cost += resources[j].R_SetupCost;
        }
    }
}

void calculate_tps(Test *test, const Condition *cond) {
    double time_demand_sum = 0.0;
    
    if (num_resources > 0) {
        time_demand_sum = test->D_Immediate[0]; 
    }
    
    double numerator = cond->W_Crit * cond->W_Surv;
    double denominator = test->Marginal_Cost + time_demand_sum;
    
    test->TPS = (denominator > 0) ? (numerator / denominator) : 0.0;
}

bool check_feasibility(const Test *test, const Resource *resources) {
    for (int j = 0; j < num_resources; j++) {
        if (resources[j].Cap_Rem < test->D_Immediate[j]) {
            return false;
        }

        if (resources[j].Cap_Total < (resources[j].Future_Demand_Commit + test->D_Future_Calc[j])) {
            return false;
        }
    }
    return true;
}

void schedule_test(int test_index, const Condition *cond) {
    Test *test = &tests[test_index];
    
    is_scheduled[test_index] = true;
    
    for (int j = 0; j < num_resources; j++) {
        resources[j].Cap_Rem -= test->D_Immediate[j];

        resources[j].Future_Demand_Commit += test->D_Future_Calc[j];
        
        if (resources[j].R_SetupCost > 0.0 && !resources[j].Utilized) {
            resources[j].Utilized = true;
            total_cost += resources[j].R_SetupCost;
        }
    }
    
    total_cost += test->C_Explicit;
    total_utility += (cond->W_Crit * cond->W_Surv);
}

int compare_tps_desc(const void *a, const void *b) {
    const Test *testA = (const Test *)a;
    const Test *testB = (const Test *)b;
    if (testA->TPS < testB->TPS) return 1;
    if (testA->TPS > testB->TPS) return -1;
    return 0;
}

int compare_demand_asc(const void *a, const void *b) {
    const Test *testA = (const Test *)a;
    const Test *testB = (const Test *)b;
    if (testA->D_Immediate[0] < testB->D_Immediate[0]) return -1;
    if (testA->D_Immediate[0] > testB->D_Immediate[0]) return 1;
    return 0;
}

void run_allocation_algorithm() {
    initialize_globals(); 
    printf("\n--- Starting Allocation for Time Slice T ---\n");
    
    for (int i = 0; i < num_tests; i++) {
        int k = tests[i].condition_k;
        
        for (int j = 0; j < num_resources; j++) {
            tests[i].D_Future_Calc[j] = tests[i].P_Pos * 1.0; 
        }

        calculate_marginal_cost(&tests[i], resources);
        calculate_tps(&tests[i], &conditions[k]);
    }

    int equity_target = (int)(T_EQUITY * resources[0].Cap_Total);
    printf("Equity Target (Min Low-Crit Slots): %d\n", equity_target);
    printf("TPS Cutoff for Low-Crit: %.2f\n", TPS_CUTOFF);

    printf("\n--- Phase 1: Equity Enforcement ---\n");
    
    Test pool_low_crit[MAX_TESTS];
    int pool_low_crit_size = 0;
    
    for (int i = 0; i < num_tests; i++) {
        if (!is_scheduled[i] && tests[i].TPS < TPS_CUTOFF) {
            pool_low_crit[pool_low_crit_size++] = tests[i];
        }
    }
    
    qsort(pool_low_crit, pool_low_crit_size, sizeof(Test), compare_demand_asc);

    for (int i = 0; i < pool_low_crit_size && equity_count < equity_target; i++) {
        int original_index = pool_low_crit[i].test_id; 

        if (check_feasibility(&tests[original_index], resources) && total_cost + tests[original_index].Marginal_Cost <= B_Max) {
            
            schedule_test(original_index, &conditions[tests[original_index].condition_k]);
            equity_count++;
            printf("  [Equity] Scheduled Test %d (TPS: %.2f) Cost: $%.2f\n", original_index, tests[original_index].TPS, total_cost);

        } else {
        }
    }
    printf("Equity Compliance: %d / %d met.\n", equity_count, equity_target);


    printf("\n--- Phase 2: Utility Maximization ---\n");
    
    Test pool_all[MAX_TESTS];
    int pool_all_size = 0;
    
    for (int i = 0; i < num_tests; i++) {
        if (!is_scheduled[i]) {
             calculate_marginal_cost(&tests[i], resources);
             calculate_tps(&tests[i], &conditions[tests[i].condition_k]);
             
             pool_all[pool_all_size++] = tests[i];
        }
    }
    
    qsort(pool_all, pool_all_size, sizeof(Test), compare_tps_desc);

    for (int i = 0; i < pool_all_size; i++) {
        int original_index = pool_all[i].test_id;
        
        if (check_feasibility(&tests[original_index], resources) && total_cost + tests[original_index].Marginal_Cost <= B_Max) {
            
            schedule_test(original_index, &conditions[tests[original_index].condition_k]);
            printf("  [Triage] Scheduled Test %d (TPS: %.2f) Cost: $%.2f\n", original_index, tests[original_index].TPS, total_cost);

        } else {
        }
    }

    printf("\n--- System Output Metrics ---\n");
    printf("Total Cost Incurred: $%.2f (Max: $%.2f)\n", total_cost, B_Max);
    printf("Total Utility Achieved: %.2f\n", total_utility);
    printf("Equity Compliance Rate: %s\n", (equity_count >= equity_target) ? "PASSED" : "FAILED");
    
    double utilization = (resources[0].Cap_Total > 0) ? 
        ((resources[0].Cap_Total - resources[0].Cap_Rem) / resources[0].Cap_Total) * 100.0 : 0.0;
    printf("Resource 0 Utilization Rate (Immediate): %.2f%%\n", utilization);

    printf("Uncovered Diagnostics (Deferred):\n");
    bool deferred = false;
    for (int i = 0; i < num_tests; i++) {
        if (!is_scheduled[i]) {
            printf("  - Test %d (Condition %d, TPS: %.2f)\n", i, tests[i].condition_k, tests[i].TPS);
            deferred = true;
        }
    }
    if (!deferred) {
        printf("  - None.\n");
    }
}

void setup_preset_data() {
    B_Max = 1000.0;
    num_resources = 2; 
    num_conditions = 3;
    num_tests = 5;

    // Conditions
    conditions[0] = (Condition){0, 80.0, 70.0}; 
    conditions[1] = (Condition){1, 20.0, 95.0}; 
    conditions[2] = (Condition){2, 50.0, 10.0}; 

    // Resources
    resources[0] = (Resource){50.0, 50.0, false, 500.0, 0.0}; 
    resources[1] = (Resource){30.0, 30.0, false, 0.0, 0.0};  

    // Tests (ID, Condition, C_Exp, P_Pos, R0 Imm, R1 Imm)
    tests[0] = (Test){0, 0, 100.0, 0.80, {10.0, 0.0}, 0.0}; 
    tests[1] = (Test){1, 1, 50.0,  0.10, {5.0,  0.0}, 0.0}; 
    tests[2] = (Test){2, 1, 150.0, 0.15, {15.0, 0.0}, 0.0}; 
    tests[3] = (Test){3, 2, 200.0, 0.90, {20.0, 0.0}, 0.0}; 
    tests[4] = (Test){4, 0, 50.0,  0.50, {5.0,  0.0}, 0.0}; 
}

void setup_user_input() {
    printf("\n*** Data Input for Allocation Algorithm ***\n");

    printf("Enter Total Budget B_Max ($): ");
    scanf("%lf", &B_Max);

    printf("Enter number of Resource Categories (Max %d): ", MAX_RESOURCES);
    scanf("%d", &num_resources);
    if (num_resources > MAX_RESOURCES) num_resources = MAX_RESOURCES;

    printf("Enter number of Health Conditions (Max %d): ", MAX_CONDITIONS);
    scanf("%d", &num_conditions);
    if (num_conditions > MAX_CONDITIONS) num_conditions = MAX_CONDITIONS;

    printf("Enter number of Diagnostic Tests (Max %d): ", MAX_TESTS);
    scanf("%d", &num_tests);
    if (num_tests > MAX_TESTS) num_tests = MAX_TESTS;
    
    printf("\n--- Condition and Diagnostic Data ---\n");
    for (int k = 0; k < num_conditions; k++) {
        conditions[k].condition_id = k;
        printf("Condition %d:\n", k);
        printf("  Criticality W_Crit (Score): ");
        scanf("%lf", &conditions[k].W_Crit);
        printf("  Survivability W_Surv (Score): ");
        scanf("%lf", &conditions[k].W_Surv);
    }
    
    printf("\n--- Resource and Capacity Data ---\n");
    for (int j = 0; j < num_resources; j++) {
        printf("Resource %d:\n", j);
        printf("  Total Capacity Cap_Total (Units/Time): ");
        scanf("%lf", &resources[j].Cap_Total);
        resources[j].Cap_Rem = resources[j].Cap_Total;
        resources[j].Utilized = false;
        resources[j].Future_Demand_Commit = 0.0;
        
        printf("  Shared Resource Setup Cost R_j ($): ");
        scanf("%lf", &resources[j].R_SetupCost);
    }

    printf("\n--- Test and Cost Data ---\n");
    for (int i = 0; i < num_tests; i++) {
        tests[i].test_id = i;
        printf("Test %d:\n", i);
        
        int k_input;
        printf("  Screens for Condition ID (0 to %d): ", num_conditions - 1);
        scanf("%d", &k_input);
        if (k_input >= 0 && k_input < num_conditions) {
             tests[i].condition_k = k_input;
        } else {
             tests[i].condition_k = 0; 
        }

        printf("  Explicit Test Cost C_i ($): ");
        scanf("%lf", &tests[i].C_Explicit);

        printf("  Prevalence P_Pos (0-1): ");
        scanf("%lf", &tests[i].P_Pos);

        for (int j = 0; j < num_resources; j++) {
            printf("  Immediate Demand D_i,%d^Immediate (Units/Time): ", j);
            scanf("%lf", &tests[i].D_Immediate[j]);
        }
    }
}

int main() {
    int choice;
    printf("Select Data Input Mode:\n");
    printf("1. Use Pre-set Example Data\n");
    printf("2. Enter Custom User Input\n");
    printf("Enter choice (1 or 2): ");
    scanf("%d", &choice);

    if (choice == 1) {
        setup_preset_data();
    } else if (choice == 2) {
        setup_user_input();
    } else {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    run_allocation_algorithm();
    return 0;
}